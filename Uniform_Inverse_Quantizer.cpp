#include <iostream>

// JUST CHANGE "#DEFINE" VALUE
#define channel 3    // R,G,B
#define width 512
#define height 512
const int AREA = height * width;            // 1 channel size
const int SIZE = channel * height * width;  // Original image size

using namespace std;

unsigned char input[SIZE], dequant[SIZE], quant[AREA];
int shift, total_bit, bpp, x, new_byte;      // Θ = 2^shift, # of bits of quantized image, bit per pixel
int main()
{
    FILE* code, * result, * origin;

    // Reconstruction
    for (int c = 0; c < channel; c++)
    {
        // Read file with byte mode
        char name[50] = "../output/Lenna_512x512_code";
        strcat_s(name, c < 1 ? "_RED" : c < 2 ? "_GREEN" : "_BLUE");
        fopen_s(&code, name, "rb");
        if (!code)
        {
            cout << "FILE OPEN ERROR\n";
            return -1;
        }
        total_bit = fread(quant, sizeof(char), AREA, code) << 3;
        fclose(code);

        // Decoding
        int bit = 0, y = 0, p_dequant = 0, p_quant = 0, i = 0;       // Bit window & temp value
        bpp += new_byte = total_bit / AREA;              // Check code len
        for (shift = 8 - new_byte; i < total_bit; i++)
        {
            if (bit == 0)    // Sliding "bits"
            {
                bit = 128;
                x = quant[p_quant++];
            }
            y += (x & bit ? 1 : 0) << --new_byte + shift;  // Cumulating value using code
            bit >>= 1;         // Move bit
            if (new_byte == 0)  // Reconstruct
            {
                dequant[c * AREA + p_dequant++] = y + (1 << shift - 1);
                new_byte = 8 - shift;
                y = 0;
            }
        }
    }

    // Make a quantized image file
    fopen_s(&result, "../output/Lenna_512x512_reconstruct.raw", "wb");
    fwrite(dequant, sizeof(char), SIZE, result);
    fclose(result);

    // Get original img
    fopen_s(&origin, "../input/Lenna_512x512_original.raw", "rb");
    if (!origin)
    {
        cout << "FILE OPEN ERROR\n";
        return -1;
    }
    fread(input, sizeof(char), SIZE, origin);
    fclose(origin);

    // Calculate Loss = 4*MSE + bits per pixel
    unsigned int MSE = 0;
    for (int i = 0; i < SIZE; i++)
    {
        x = input[i];
        MSE += (x - dequant[i]) * (x - dequant[i]);
    }
    printf("Reconstruction OK.\nTotal Loss = %lf", MSE * 4.0 / SIZE + bpp);

    return 0;
}
