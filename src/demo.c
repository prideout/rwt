#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void par_bluenoise_create(const char * fileName);
float* par_bluenoise_generate(float x, float y, float z, int* npts);
void par_bluenoise_set_density(const unsigned char* pixels, int size);

static int clampi(int x, int min, int max)
{
	if (x < min) x = min;
	if (x > max) x = max;
	return x;
}

static int sqri(int a) { return a*a; };

static void drawPoints(const char* dstfile, float vx, float vy, float vz)
{
	int numPoints;
    float* pts = par_bluenoise_generate(vx, vy, vz, &numPoints);
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
    par_bluenoise_create("bluenoise.bin");
    int dims[3];
    stbi_uc* data = stbi_load("trillium.jpg", &dims[0], &dims[1], &dims[2], 1);
	assert(dims[0] == dims[1]);
	par_bluenoise_set_density(data, dims[0]);
    stbi_image_free(data);
    drawPoints("build/output_01.png", 0, 0, 1);
    drawPoints("build/output_02.png", 0.25f, 0.25f, 0.5f);
    drawPoints("build/output_03.png", 0.45f, 0.45f, 0.1f);
    return 0;
}
