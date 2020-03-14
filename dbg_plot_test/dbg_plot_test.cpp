#include "dbg_plot/dbg_plot.h"

#if 1
#define dbg_msg printf
#else
//TODO: double check this implementation
#define dbg_msg(__x__) while(0) __x__;
#endif

void DemonstrateDbgPlot()
{
    DbgPlot::DbgPlot dp("C:/HHWork/LW3D/Development/Data/Output/DbgPlotTest");

    // Try out cos, sin, exp
    {
        dbg_msg("\n Verify y plotting \n");

        float damped_osc_val[64];
        const float pi = 3.14159f;

        for (int i = 0; i < 64; i++)
        {
            float i_f = i / 64.0f * 5;
            damped_osc_val[i] = std::cos(2.0f * pi * i_f) * (1.0f / std::exp(i_f));
        }

        dp["hello figure"].subplots(1,2);

        dp["hello figure"].suptitle("hello figure");

        dp["hello figure"][0].plot(damped_osc_val, 64, "-b");
        dp["hello figure"][0].set_title("Damped Oscillation");
        dp["hello figure"][0].set_xlabel("distance (m)");
        dp["hello figure"][0].set_ylabel("Amplitude");

        float sinc_val[128];
        for (int i = 0; i < 128; i++)
        {
            float x = 0.3f * (i - 64) / 2;
            if (x != 0.0f)
                sinc_val[i] = std::sin(x) / x;
            else
                sinc_val[i] = 1.0f;
        }

        dp["hello figure"][1].plot(sinc_val, 128, "--r");
        dp["hello figure"][1].set_title("Sinc");
    }

    // Display two dimensional (NxM) axes
    {
        dbg_msg("\n Verify NxM plots \n");

        short short_val[6][80];
        for (int j=0; j<6; j++)
            for (int i = 0; i < 80; i++)
                short_val[j][i] = static_cast<short>(i*(j+1));

        dp["figure 2d"].subplots(3, 2);
        dp["figure 2d"][0].plot(&short_val[0][0], 80);
        dp["figure 2d"][1].plot(&short_val[1][0], 80);
        dp["figure 2d"][2].plot(&short_val[2][0], 80);
        dp["figure 2d"][3].plot(&short_val[3][0], 80);
        dp["figure 2d"][4].plot(&short_val[4][0], 80);
        dp["figure 2d"][5].plot(&short_val[5][0], 80);

        dp["figure 2d 2"].subplots(2, 2);
        dp["figure 2d 2"][{0, 0}].plot(&short_val[0][0], 80);
        dp["figure 2d 2"][{1, 0}].plot(&short_val[1][0], 80);
        dp["figure 2d 2"][{0, 1}].plot(&short_val[0][1], 80);
        dp["figure 2d 2"][{1, 1}].plot(&short_val[1][1], 80);
    }

    // Plot x, y
    {
        dbg_msg("\n Verify 2D plottting functionality (1)\n");

        dp["plot xy"].subplots(2, 1);
        int x_tri[] = { 0, 1, 3, 5, 7, 9, 11 };
        int y_tri[] = { 0, 3, 4, 6, 4, 3, 0 };
        dp["plot xy"][0].plot(x_tri, y_tri, 7);

        int x[] = { 242, 319, 280, 242, 308 };
        int y[] = { 257, 257, 296, 340, 342 };
        dp["plot xy"][1].plot(x, y, 5, "ob");
        dp["plot xy"][1].set_title("Face Location");
    }
    
    // imshow functionality verification
    {
        dbg_msg("\n Verify imshow functionality\n");

        dp["hello imshow"].subplots(1, 2);
        dp["hello imshow"][0].imshow(
            "../stinkbug.png", 
            "cmap=\"gray\"");
        dp["hello imshow"][1].imshow(
            "../stinkbug.png", 
            150, 200, 100, 100, 
            "cmap=\"gray\"");
        dp["hello imshow"][0].add_patch(
            { 150, 200,100,100 },
            "linewidth=1, edgecolor = \"y\", fill=None"
            );
        dp["hello imshow"][0].text(
            {150, 200-8}, 
            "cropped region", 
            "fontsize=8, horizontalalignment = \"left\", verticalalignment = \"center\", color = \"yellow\""
            );
    }

    // imshow with built-in array verification
    {
        dbg_msg("\n Verify imshow built-in array functionality\n");

        // checker board pattern
        const int width = 100;
        const int height = 100;
        uint8_t* buf = new uint8_t[width * height];
        uint8_t* p = buf;
        for (int j = 0; j < height; j++)
        {
            for (int i = 0; i < width; i++)
            {
                if (((i / 20) % 2 == 0 && ((j / 20) % 2) == 0) ||
                    ((i / 20) % 2 == 1 && ((j / 20) % 2) == 1))
                {
                    *p = 0x00;
                }
                else
                {
                    *p = 0xFF;
                }
                p++;
            }
        }

        dp["checker board"].subplots();
        dp["checker board"][0].imshow(
            "checker board image",
            buf,
            width,
            height,
            width,
            "gray",
            { 10, 10, 60, 60 });

        delete[] buf;
    }
}

void DemonstrateDbgListText()
{
    DbgPlot::DbgListText dtxt("C:/HHWork/LW3D/Development/Data/Output/DbgListTextTest");
    
    std::string a("hello text list");
    const int x[] = { 334, 413, 373, 342, 402 };
    const int y[] = { 221, 222, 260, 304, 305 };
    dtxt[a].add("x locations", x, 5);
    dtxt[a].add("y locations", y, 5);
}

int main(int argc, char* argv)
{
    DemonstrateDbgPlot();

    DemonstrateDbgListText();

    return 0;
}