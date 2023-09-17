#include <iostream>
#include <bitset>    // std::bitset
#include <cstring>

// JUST CHANGE "#DEFINE" VALUE
#define channel 3    // R,G,B
#define width 512
#define height 512
const int AREA = height * width;            // 1 Channel size
const int SIZE = channel * height * width;  // Original image size

const char ch[3] = { 'R', 'G', 'B' };   // R, G, B code
unsigned char input[SIZE], quant[AREA], x;

using namespace std;

int main()
{
    bitset<8> code;    // 1 byte
    double loss;
    int code_len, i, s, min, max;
    int map[256], codebook[128];    // Result mapping value & codebook
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

    // Scalar Non-Uniform Quantization
    for (int c = 0; c < channel; c++)
    {
        // Initialize
        code_len = 8;
        loss = 24;
        min = 256;
        max = -1;
        int count[256] = { 0, };
        for (i = 0; i < AREA; i++)
        {
            x = input[c * AREA + i];
            count[x]++;
            min = min > x ? x : min;
            max = max < x ? x : max;
        }

        for (int len = 8; --len;)        // Iterative for optimized code len
        {
            double d[129], r[128];
            int sub_count = 0, temp_r = 0, level = 1;

            // Level setting
            /* // Count oriented level
            for (i = min; i < max && level < 1 << len; i++)
            {
                sub_count += count[i];
                while (sub_count >= AREA >> len)
                {
                    d[level++] = i;
                    sub_count -= AREA >> len;
                }
            }*/
            // Uniform level
            for (i = 1; i < 1 << len; i++)
                d[i] = (double)i * (max - min) / (1 << len) + min;
            d[0] = min;             // Minimum
            d[1 << len] = max;      // Maximum

            // Search optimized decision & reconstruction level
            for (int iter = 0; iter < 7; iter++)   // Just 7 iteration
            {
                s = min;
                // Update reconstruction level : r = ¥Òs*p(s)/¥Òp(s)
                for (i = 1; i <= 1 << len; i++)
                {
                    for (sub_count = temp_r = 0; s <= d[i] && s <= max; s++)
                    {
                        sub_count += count[s];
                        temp_r += s * count[s];
                    }
                    r[i - 1] = sub_count ? (double)temp_r / sub_count : i < 1 << len - 1 ? d[i] : d[i - 1];
                }

                // Update decision level : d = (r_i+r_i+1)/2
                for (i = 1; i < 1 << len; i++)
                    d[i] = (r[i - 1] + r[i]) / 2;
            }

            // Set current mapping value & codebook
            int cur_map[256], cur_codebook[128];
            for (i = min, s = 0; i <= max; cur_map[i++] = s - (s >= 1 << len))
                while (s < 1 << len && i >= d[s + 1])
                {
                    cur_codebook[s] = r[s] + 0.5 <= d[s + 1] ? r[s] + 0.5 : r[s];
                    s++;
                }
            /* // Scatter duplicate code
            for (s = 0; s + 1 < 1 << len; s++)
            {
                for (i = 1; i + s < 1 << len && cur_codebook[s] == cur_codebook[i + s]; i++);
                if (i > 1)
                {
                    if (s && cur_codebook[s] - cur_codebook[s - 1] > cur_codebook[s + i] - cur_codebook[s + i - 1])
                        cur_codebook[s]--;
                    else
                        cur_codebook[s + i - 1]++;
                }
            }
            */

            // Loss calculate
            unsigned int sum = 0;
            for (int p = 0; p < AREA; p++)
            {
                x = input[c * AREA + p];
                sum += (x - cur_codebook[cur_map[x]]) * (x - cur_codebook[cur_map[x]]);
            }
            double cur_loss = (4.0 * sum) / SIZE + len + 16;
            printf("%c channel %d bit quantize Loss : %lf\n", ch[c], len, cur_loss);

            // Loss compare
            if (cur_loss < loss)
            {
                loss = cur_loss;
                code_len = len;
                memcpy(map, cur_map, sizeof(cur_map));
                memcpy(codebook, cur_codebook, sizeof(int) * (1 << len));
            }
        }
        printf("-> %c channel will be quantized to %d bit\n\n", ch[c], code_len);
        
        const int total_bit = AREA * code_len;   // # of bits of quantized image
        const int total_byte = total_bit >> 3;    // # of bytes of quantized image
        int p_input = 0, p_quant = 0;

        // Make a 'code'
        for (int i = 0, idx = 0; i < total_bit; i++)
        {
            if (idx == 0)        // Get next pixel
            {
                x = map[input[c * AREA + p_input++]];
                idx = code_len;
            }
            code[(~i) & 7] = (x & (1 << --idx)) > 0;  // binary shifting
            if ((i & 7) == 7)  // 8 bits = 1 byte
                quant[p_quant++] = code.to_ulong();   // Made one "byte"
        }

        // Make a quantized image file
        char name[50] = "../output/Lenna_512x512_code";
        strcat_s(name, c < 1 ? "_RED" : c < 2 ? "_GREEN" : "_BLUE");
        fopen_s(&result, name, "wb");
        fwrite(quant, sizeof(char), total_byte, result);
        fclose(result);

        // Make codebook file
        strcpy_s(name, "../output/Lenna_512x512_codebook");
        strcat_s(name, c < 1 ? "_RED" : c < 2 ? "_GREEN" : "_BLUE");
        fopen_s(&result, name, "w");
        for(i = 0; i < 1 << code_len; i++)
            fprintf(result, "%d\n", codebook[i]);
        fclose(result);
    }

    return 0;
}