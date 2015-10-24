#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

typedef struct par_bluenoise_context_s par_bluenoise_context;
par_bluenoise_context* par_bluenoise_create(const char* file_or_data,
	int data_length);
void par_bluenoise_free(par_bluenoise_context* ctx);
void par_bluenoise_set_density(par_bluenoise_context* ctx,
	const unsigned char* pixels, int size);
float* par_bluenoise_generate(par_bluenoise_context* ctx, float x, float y,
	float z, int* npts);

static int clampi(int x, int min, int max)
{
	if (x < min) x = min;
	if (x > max) x = max;
	return x;
}

static int sqri(int a) { return a*a; };

static void drawPoints(par_bluenoise_context* ctx, const char* dstfile, float vx, float vy, float vz)
{
	int numPoints;
    float* pts = par_bluenoise_generate(ctx, vx, vy, vz, &numPoints);
    const int size = 512;
    unsigned char* buffer = (unsigned char*) malloc(sqri(size));
    for (int i = 0; i < sqri(size); i++) {
        buffer[i] = 255;
    }
    while (numPoints--) {
        float x = (*pts++ - vx) / vz;
        float y = (*pts++ - vy) / vz;
        int i = clampi(x * size, 0, size - 1);
        int j = clampi(y * size, 0, size - 1);
        assert(i >= 0 && i < size);
        assert(j >= 0 && j < size);
        buffer[i + j * size] = 0;
    }
    stbi_write_png(dstfile, size, size, 1, buffer, size);
    free(buffer);
    printf("Write %s\n", dstfile);
}

int main()
{
	par_bluenoise_context* ctx;
    int dims[3];
	stbi_uc* data;
    ctx = par_bluenoise_create("bluenoise.bin", 0);
    data = stbi_load("trillium.jpg", &dims[0], &dims[1], &dims[2], 1);
	assert(dims[0] == dims[1]);
	par_bluenoise_set_density(ctx, data, dims[0]);
    stbi_image_free(data);
    drawPoints(ctx, "build/output_01.png", 0, 0, 1);
    drawPoints(ctx, "build/output_02.png", 0.25f, 0.25f, 0.5f);
    drawPoints(ctx, "build/output_03.png", 0.45f, 0.45f, 0.1f);
	par_bluenoise_free(ctx);
    return 0;
}
