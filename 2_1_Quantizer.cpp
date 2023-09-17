#include <iostream>
#include <bitset>    // std::bitset
#include <cstring>

// JUST CHANGE "#DEFINE" VALUE
#define channel 3    // R,G,B
#define width 512
#define height 512
const int AREA = height * width;            // 1 channel size
const int SIZE = channel * height * width;  // Original image size

using namespace std;

const char ch[3] = { 'R', 'G', 'B' };   // R, G, B code
unsigned char input[SIZE], quant[AREA], x;

int main()
{
    bitset<8> code;    // 1 byte
    int shift;    // Pixels of images
    double loss;
    FILE* origin, * result;

    // Read file with byte mode
    fopen_s(&origin, "../input/Lenna_512x512_original.raw", "rb");
    if (!origin)
    {
        cout << "FILE OPEN ERROR\n";
        return -1;
    }
    fread(input, sizeof(char), SIZE, origin);
    fclose(origin);

    // Scalar Uniform Quantization
    for (int c = 0; c < channel; c++)
    {
        shift = 0;                            // Not quantize
        loss = 24;
        for (int sh = 1; sh < 8; sh++)        // Iterative for optimized code len
        {
            // Loss calculate
            unsigned int sum = 0;
            for (int p = 0; p < AREA; p++)
            {
                x = input[c * AREA + p];
                sum += (x - ((x >> sh) << sh) - (1 << sh - 1)) * (x - ((x >> sh) << sh) - (1 << sh - 1));   // Uniform Quantization
            }
            double cur_loss = (sum * 4.0) / SIZE + 24 - sh; // Total Loss when 1 channel is quantized
            printf("%c channel %d bit quantize Loss : %lf\n", ch[c], 8 - sh, cur_loss);

            // Loss compare
            if (cur_loss < loss)
            {
                loss = cur_loss;
                shift = sh;
            }
        }
        printf("-> %c channel will be quantized to %d bit\n\n", ch[c], 8 - shift);

        const int total_bit = AREA * (8 - shift);   // # of bits of quantized image
        const int total_byte = total_bit >> 3;    // # of bytes of quantized image
        int p_input = 0, p_quant = 0;

        // Iteration 
        for (int i = 0, idx = 0; i < total_bit; i++)
        {
            // Get a data of pixel
            if (idx < shift)        // Some value will be cut-off
            {
                x = input[c * AREA + p_input++];
                idx = 7;
            }
            code[(~i) & 7] = (x & (1 << idx--)) > 0;  // binary shifting
            if ((i & 7) == 7)  // 8 bits = 1 byte
                quant[p_quant++] = code.to_ulong();   // Made one "byte"
        }

        // Make a quantized image file
        char name[50] = "../output/Lenna_512x512_code";
        strcat_s(name, c < 1 ? "_RED" : c < 2 ? "_GREEN" : "_BLUE");
        fopen_s(&result, name, "wb");
        fwrite(quant, sizeof(char), total_byte, result);
        fclose(result);
    }

    return 0;
}