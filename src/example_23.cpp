// CPU Path Tracing (https://raytracing.github.io/)

#include "macros.h"
#include "entry.h"

#include <vector>
#include <random>

#include "vec3.h"
#include "vec4.h"
#include "mat4.h"
#include "ray3.h"

#include <chrono>
using namespace std::chrono;

#include <thread>

#define MAX_SAMPLES_PER_PIXEL 200
#define RAY_DEPTH 6

#define M_PI  3.1415926536f
#define M_2PI 6.2831853072f

const char * renderVertexShaderSource = R"(
#version 410 core

out vec2 vTexCoord;

void main()
{
	vTexCoord = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
	gl_Position = vec4((vTexCoord*2.0 - 1.0), 0.0, 1.0);
}
)";

const char * renderFragmentShaderSource = R"(
#version 410 core

#define INV_SQRT_OF_2PI 0.39894228040143267793994605993439  // 1.0/SQRT_OF_2PI
#define INV_PI 0.31830988618379067153776752674503

//  smartDeNoise - parameters
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  sampler2D tex     - sampler image / texture
//  vec2 uv           - actual fragment coord
//  float sigma  >  0 - sigma Standard Deviation
//  float kSigma >= 0 - sigma coefficient
//      kSigma * sigma  -->  radius of the circular kernel
//  float threshold   - edge sharpening threshold

vec4 smartDeNoise(sampler2D tex, vec2 uv, float sigma, float kSigma, float threshold)
{
    float radius = round(kSigma*sigma);
    float radQ = radius * radius;

    float invSigmaQx2 = .5 / (sigma * sigma);      // 1.0 / (sigma^2 * 2.0)
    float invSigmaQx2PI = INV_PI * invSigmaQx2;    // 1.0 / (sqrt(PI) * sigma)

    float invThresholdSqx2 = .5 / (threshold * threshold);     // 1.0 / (sigma^2 * 2.0)
    float invThresholdSqrt2PI = INV_SQRT_OF_2PI / threshold;   // 1.0 / (sqrt(2*PI) * sigma)

    vec4 centrPx = texture(tex,uv);

    float zBuff = 0.0;
    vec4 aBuff = vec4(0.0);
    vec2 size = vec2(textureSize(tex, 0));

    for(float x=-radius; x <= radius; x++) {
        float pt = sqrt(radQ-x*x);  // pt = yRadius: have circular trend
        for(float y=-pt; y <= pt; y++) {
            vec2 d = vec2(x,y);

            float blurFactor = exp( -dot(d , d) * invSigmaQx2 ) * invSigmaQx2PI;

            vec4 walkPx =  texture(tex,uv+d/size);

            vec4 dC = walkPx-centrPx;
            float deltaFactor = exp( -dot(dC, dC) * invThresholdSqx2) * invThresholdSqrt2PI * blurFactor;

            zBuff += deltaFactor;
            aBuff += deltaFactor*walkPx;
        }
    }
    return aBuff/zBuff;
}

uniform sampler2D texture0;

in vec2 vTexCoord;

out vec4 FragColor;

void main()
{
	// FragColor = texture(texture0, vTexCoord);
	FragColor = smartDeNoise(texture0, vTexCoord, 2.0, 2.0, 0.08);
}
)";

std::random_device rd;
std::default_random_engine re(rd());
std::uniform_real_distribution<float> two_pi_real_dist(0.0f, M_2PI);
std::uniform_real_distribution<float> real_dist(0.0f, 1.0f);

GLuint gProgram;
GLuint gVAO;
GLuint gTexture;

int gRenderWidth = 0;
int gRenderHeight = 0;

int gSamples = 0;
unsigned char* gPixels = nullptr;
vec3* gStoragePixels = nullptr;

unsigned int gCores;
std::thread* gThreads = nullptr;

float gFpsTime = 0.0f;
int gFpsCount = 0;

vec3 gEyePos;
vec3 gPrevEyePos;
double gPrevPosX;
double gPrevPosY;
float gTargetRotX;
float gTargetRotY;
float gRotX;
float gRotY;

inline float clampf(float x, float min, float max) {
	if (x < min) return min;
	if (x > max) return max;
	return x;
}

inline vec3 random_in_unit_sphere()
{
	// https://datagenetics.com/blog/january32020/index.html
	float theta = two_pi_real_dist(re);
	float v = real_dist(re);
	float phi = acosf((2.0f*v) - 1.0f);
	float r = powf(real_dist(re), 1.0f/3.0f);
	return vec3(r*sinf(phi)*cos(theta), r*sinf(phi)*sin(theta), r*cos(phi));
}

struct hit
{
	vec3 point{};
	vec3 normal{};
	float t{};
	bool frontFace{};
	vec3 color{};

	// -- Implicit basic constructors --
	constexpr hit() = default;
	constexpr hit(hit const& s) = default;

	inline void faceNormal(const ray3& r, const vec3& outwardNormal)
	{
		frontFace = vec3::dot(r.direction, outwardNormal) < 0.0f;
		normal = frontFace ? outwardNormal : -outwardNormal;
	}
};

struct lambertianSphere
{
	vec3 center{};
	float radius{};
	vec3 color{};

	// -- Implicit basic constructors --
	constexpr lambertianSphere() = default;
	constexpr lambertianSphere(lambertianSphere const& s) = default;

	// -- Explicit basic constructors --
	inline constexpr lambertianSphere(const vec3& center, float radius, const vec3& color) :
		center(center), radius(radius), color(color)
	{
	}
};

struct metalSphere
{
	vec3 center{};
	float radius{};
	vec3 color{};
	float fuzz{};

	// -- Implicit basic constructors --
	constexpr metalSphere() = default;
	constexpr metalSphere(metalSphere const& s) = default;

	// -- Explicit basic constructors --
	inline constexpr metalSphere(const vec3& center, float radius, const vec3& color, float fuzz) :
		center(center), radius(radius), color(color), fuzz(fuzz)
	{
	}
};

std::vector<lambertianSphere> lambertianSpheres =
{
	lambertianSphere(vec3(-1.0f, 0.5f, 0.0f), 0.5f, vec3(1.0f, 0.5f, 0.5f)),
	lambertianSphere(vec3(0.0f, 0.5f, 0.0f), 0.5f, vec3(0.6f, 0.6f, 0.6f)),
	lambertianSphere(vec3(1.0f, 0.5f, 0.0f), 0.5f, vec3(0.5f, 0.5f, 1.0f)),
	lambertianSphere(vec3(1.0f, 1.0f, -2.0f), 1.0f, vec3(0.5f, 1.0f, 0.5f)),
	lambertianSphere(vec3(0.0f, -100.0f, 0.0f), 100.0f, vec3(0.5f, 0.5f, 0.5f))
};

std::vector<metalSphere> metalSpheres =
{
	metalSphere(vec3(-2.0f, 0.5f, 0.0f), 0.5f, vec3(0.8f, 0.8f, 0.8f), 0.1f),
	metalSphere(vec3(2.0f, 0.5f, 0.0f), 0.5f, vec3(0.8f, 0.8f, 0.8f), 0.1f),
	metalSphere(vec3(-1.0f, 1.0f, 2.0f), 1.0f, vec3(0.8f, 0.8f, 0.8f), 0.1f)
};

bool hitLambertianSphere(const ray3& r, const lambertianSphere& s, float minT, float maxT, hit& hit, vec3& scatterDirection)
{
	vec3 oc = r.origin - s.center;
	float a = vec3::dot(r.direction, r.direction);
	float halfb = vec3::dot(oc, r.direction);
	float c = vec3::dot(oc, oc) - s.radius*s.radius;
	float discriminant = halfb*halfb - a*c;
	if (discriminant < 0.0f) return false;

	float sqrtd = sqrtf(discriminant);

	float root = (-halfb - sqrtd)/a;
	if (root < minT || maxT < root)
	{
		float root = (-halfb + sqrtd)/a;
		if (root < minT || maxT < root)
		{
			return false;
		}
	}

	hit.t = root;
	hit.point = r.at(root);
	vec3 outwardNormal = (hit.point - s.center)/s.radius;
	hit.faceNormal(r, outwardNormal);
	hit.color = s.color;

	{
		vec3 randomUnitSphere = random_in_unit_sphere();

		scatterDirection = hit.normal + randomUnitSphere.normalize();
		if ((fabsf(scatterDirection.x) < 1e-6f) && (fabsf(scatterDirection.y) < 1e-6f) && (fabsf(scatterDirection.z) < 1e-6f))
		{
			scatterDirection = hit.normal;
		}
	}

	return true;
}

bool hitMetalSphere(const ray3& r, const metalSphere& s, float minT, float maxT, hit& hit, vec3& scatterDirection)
{
	vec3 oc = r.origin - s.center;
	float a = vec3::dot(r.direction, r.direction);
	float halfb = vec3::dot(oc, r.direction);
	float c = vec3::dot(oc, oc) - s.radius*s.radius;
	float discriminant = halfb*halfb - a*c;
	if (discriminant < 0.0f) return false;

	float sqrtd = sqrtf(discriminant);

	float root = (-halfb - sqrtd)/a;
	if (root < minT || maxT < root)
	{
		float root = (-halfb + sqrtd)/a;
		if (root < minT || maxT < root)
		{
			return false;
		}
	}

	hit.t = root;
	hit.point = r.at(root);
	vec3 outwardNormal = (hit.point - s.center)/s.radius;
	hit.faceNormal(r, outwardNormal);
	hit.color = s.color;

	scatterDirection = r.direction - hit.normal*2.0f*vec3::dot(r.direction, hit.normal) + random_in_unit_sphere()*s.fuzz;
	return vec3::dot(scatterDirection, hit.normal) > 0.0f;
}

vec3 rayColor(const ray3& r, int depth)
{
	if (depth <= 0) return vec3();

	hit hit;

	bool isHit = false;
	float closestSoFar = 1000.0f;
	vec3 scatterDirection;
	for (const auto& s : lambertianSpheres)
	{
		if (hitLambertianSphere(r, s, 0.001f, closestSoFar, hit, scatterDirection) == true)
		{
			isHit = true;
			closestSoFar = hit.t;
		}
	}
	for (const auto& s : metalSpheres)
	{
		if (hitMetalSphere(r, s, 0.001f, closestSoFar, hit, scatterDirection) == true)
		{
			isHit = true;
			closestSoFar = hit.t;
		}
	}

	if (isHit == true)
	{
		vec3 target = hit.point + scatterDirection;
		return rayColor(ray3(hit.point, target - hit.point), depth - 1)*hit.color;
		// return vec3(hit.normal.x + 1.0f, hit.normal.y + 1.0f, hit.normal.z + 1.0f)*0.5f;
	}

	return vec3(0.5f, 0.7f, 1.0f);
}

void render(int start, int stop, const vec3& origin, const vec3& horizontal, const vec3& vertical, const vec3& lowerLeftCorner)
{
	for (int j = start; j < stop; ++j)
	{
		for (int i = 0; i < gRenderWidth; ++i)
		{
			float u = (i + real_dist(re)) / (gRenderWidth - 1);
			float v = (j + real_dist(re)) / (gRenderHeight - 1);
			ray3 r(origin, lowerLeftCorner + horizontal*u + vertical*v - origin);

			int storageIndex = (i + j*gRenderWidth);
			vec3 pixelColor(gStoragePixels[storageIndex]);
			pixelColor = pixelColor + rayColor(r, RAY_DEPTH);

			int index = (i + j*gRenderWidth)*4;
			gPixels[index + 0] = (int)(256*clampf(sqrtf(pixelColor.x/gSamples), 0.0f, 0.999f));
			gPixels[index + 1] = (int)(256*clampf(sqrtf(pixelColor.y/gSamples), 0.0f, 0.999f));
			gPixels[index + 2] = (int)(256*clampf(sqrtf(pixelColor.z/gSamples), 0.0f, 0.999f));
			gPixels[index + 3] = 255;

			gStoragePixels[storageIndex] = pixelColor;
		}
	}
}

auto init() -> bool
{
	{
		auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &renderVertexShaderSource, nullptr);
		GL_CHECK(glCompileShader(vertexShader));
		{
			GLint success;
			GL_CHECK(glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success));
			if (!success)
			{
				GLint infoLength = 0;
				glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLength);
				char* infoLog = new char[infoLength];
				GL_CHECK(glGetShaderInfoLog(vertexShader, infoLength, &infoLength, infoLog));
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << "VERTEX_SHADER" << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
				delete[] infoLog;
				GL_CHECK(glDeleteShader(vertexShader));
			}
		}

		auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &renderFragmentShaderSource, nullptr);
		GL_CHECK(glCompileShader(fragmentShader));
		{
			GLint success;
			GL_CHECK(glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success));
			if(!success)
			{
				GLint infoLength = 0;
				glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &infoLength);
				char* infoLog = new char[infoLength];
				GL_CHECK(glGetShaderInfoLog(fragmentShader, infoLength, &infoLength, infoLog));
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << "FRAGMENT_SHADER" << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
				delete [] infoLog;
				GL_CHECK(glDeleteShader(fragmentShader));
			}
		}

		gProgram = glCreateProgram();
		glAttachShader(gProgram, vertexShader);
		glAttachShader(gProgram, fragmentShader);
		glLinkProgram(gProgram);

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

	glGenVertexArrays(1, &gVAO);

	glGenTextures(1, &gTexture);
	glBindTexture(GL_TEXTURE_2D, gTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		// glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, gRenderWidth, gRenderHeight);

	on_size();

	return true;
}

void on_size()
{
	gSamples = 0;
	gRenderWidth = gWidth / 2;
	gRenderHeight = gHeight / 2;

	if (gStoragePixels != nullptr)
	{
		delete [] gStoragePixels;
	}
	if (gPixels != nullptr)
	{
		delete [] gPixels;
	}

	gStoragePixels = new vec3[gRenderWidth*gRenderHeight];
	memset(gStoragePixels, 0, gRenderWidth*gRenderHeight*sizeof(vec3));

	gPixels = new unsigned char[gRenderWidth*gRenderHeight*4];
	memset(gPixels, 0, gRenderWidth*gRenderHeight*4);

	glDeleteTextures(1, &gTexture);
	glGenTextures(1, &gTexture);
	glBindTexture(GL_TEXTURE_2D, gTexture);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, gRenderWidth, gRenderHeight);
	glViewport(0, 0, gWidth, gHeight);
}

void on_key(int key, int action)
{

}

void on_mouse(double xpos, double ypos)
{
	int state = glfwGetMouseButton(g_pWindow, GLFW_MOUSE_BUTTON_LEFT);
	if (state == GLFW_PRESS)
	{
		gTargetRotX += (xpos - gPrevPosX)*0.01f;
		gTargetRotY += (ypos - gPrevPosY)*0.01f;

		if (gTargetRotY <= -M_PI / 2.0f) gTargetRotY = -M_PI / 2.0f + 0.01f;
		if (gTargetRotY >= M_PI / 2.0f)  gTargetRotY = M_PI / 2.0f - 0.01f;
	}
	gPrevPosX = xpos;
	gPrevPosY = ypos;
}

auto update() -> void
{
	if (fabs(gTargetRotX - gRotX) < 0.01f)
	{
		gRotX = gTargetRotX;
	}
	else
	{
		gRotX += 0.05f * (gTargetRotX - gRotX);
	}

	if (fabs(gTargetRotY - gRotY) < 0.01f)
	{
		gRotY = gTargetRotY;
	}
	else
	{
		gRotY += 0.05f * (gTargetRotY - gRotY);
	}

	vec4 eyePos = mat4::rotate(0.0f, 1.0f, 0.0f, -gRotX) * mat4::rotate(1.0f, 0.0f, 0.0f, -gRotY) * vec4(0.0f, 0.0f, 7.0f, 0.0f);
	gEyePos = vec3(eyePos.x, eyePos.y, eyePos.z);
	if (gEyePos != gPrevEyePos)
	{
		gSamples = 0;
		memset(gStoragePixels, 0, gRenderWidth*gRenderHeight*sizeof(vec3));
		memset(gPixels, 0, gRenderWidth*gRenderHeight*4);
	}
	gPrevEyePos = gEyePos;

	if (gSamples > MAX_SAMPLES_PER_PIXEL) return;

	auto start = high_resolution_clock::now();

	// Camera
	float aspect = (float)gRenderWidth / gRenderHeight;
	float h = tanf(0.7853f/2.0f);
	float viewportHeight = 2.0f*h;
	float viewportWidth = aspect*viewportHeight;
	float focalLength = 1.0f;
	//vec3 origin = vec3(0.0f, 0.0f, -5.0f);
	vec3 origin = gEyePos;
	vec3 lookAt = vec3(0.0f, 0.0f, 0.0f);
	vec3 w = (origin - lookAt).normalize();
	vec3 u = vec3::cross(vec3(0.0f, 1.0f, 0.0f), w).normalize();
	vec3 v = vec3::cross(w, u);

	vec3 horizontal = u*viewportWidth;
	vec3 vertical = v*viewportHeight;
	vec3 lowerLeftCorner = origin - horizontal/2.0f - vertical/2.0f - w;

	// Thread rendering
	++gSamples;
	int step = gCores*2 > gRenderHeight ? gRenderHeight : gCores*2;
	int tasksPerThread = (gRenderHeight + step - 1)/step;
	for (int i = 0; i < step; ++i)
	{
		int start = i*tasksPerThread;
		int stop = (i+1)*tasksPerThread;
		if (stop > gRenderHeight)
		{
			stop = gRenderHeight;
		}
		gThreads[i] = std::thread(render, start, stop, origin, horizontal, vertical, lowerLeftCorner);
	}

	for (int i = 0; i < step; ++i)
	{
		gThreads[i].join();
	}

	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);

	gFpsTime += duration.count() / 1000000.0f;
	++gFpsCount;
	if (gFpsCount > 30)
	{
		std::cout << "FPS: " << gFpsCount / gFpsTime << " - " << gSamples << std::endl;
		gFpsTime = 0;
		gFpsCount = 0;
	}

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, gRenderWidth, gRenderHeight, GL_RGBA, GL_UNSIGNED_BYTE, gPixels);
}

auto draw() -> void
{
	glUseProgram(gProgram);
	glBindVertexArray(gVAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

auto main() -> int
{
	gCores = std::thread::hardware_concurrency();
	std::cout << gCores << " concurrent threads are supported" << std::endl;

	gThreads = new std::thread[gCores*2];

	int result = run();

	if (gStoragePixels != nullptr)
	{
		delete [] gStoragePixels;
	}
	if (gPixels != nullptr)
	{
		delete [] gPixels;
	}
	delete [] gThreads;

	return result;
}
