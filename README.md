How to use:

    using namespace DbgPlot;

    std::string s = "fig n_y_data vs n_y_data_up";

    dp.setfolder("../../HtLi/trail1/");
    dp[s].axes(2,2);
    dp[s][0].plot(ptr, len, "r-");
    dp[s][1].imshow(ptr, width, height, "properties");
    dp[s][1].rectangle(x, y, w, h, "properties");
    dp[s][2].text(x, y, "this is rectangle", "properties");

    you can do following both
    dp[s][{2,1}].plot(ptr, len, "r-");
    dp[s][2].plot(ptr, len, "r-");
}

should I do:
dp["hello figure"].subplots(3,5);
dp["hello figure"][2][4].plot(...)
or 
dp["hello figure"][8].plot(...)

Do the imshow, then the rectangle

short* img_ptr;
int* img_ptr;
uint8_t* img_ptr;

dp::image im(img_ptr[4], type_name=rgb_interleaved, {width, height})
dp::image im(img_ptr[4], type_name=gray, {width, height})

dp["figure image"][0].imshow(img_ptr[4], dp::img_gray, {width, height});
dp["figure image"][0].imshow(img_ptr[4], dp::img_rgb_interleaved, {width, height});
dp["figure image"][0].imshow(img_ptr[4], dp::img_rgb_planar, {width, height})
dp["figure image"][0].imshow(img_ptr[4], dp::img_yuv_nv21, {width, height});

dp["figure image"][0].imshow("./hello.png");

{
    if(width*height>10240)
        write_binary_with_typeid
        typeid(T).name
        imread
        imshow
    else
        array_2d=[...]
        imshow(array_2d)
}
nir_diff = nir_landmark.CalcAllDiff
nir_diff = nir_landmark.CalcAllDiff

    struct Loc
    {
        Loc() { xy[0] = 0; xy[1] = 0; }
        Loc(const int ix, const int iy) { xy[0] = ix; xy[1] = iy; }
        void operator()(const int ix, const int iy) { xy[0] = ix; xy[1] = iy; }
        Loc(const Loc& rhs) { this->xy[0] = rhs.xy[0]; this->xy[1] = rhs.xy[1]; }
        Loc& operator=(const Loc& rhs) { *this = rhs; return *this; }
        Loc& operator=(const int ixy[2]) { xy[0] = ixy[0]; xy[1] = ixy[1]; }
        int& operator[](const int idx) { return xy[idx]; }
        int xy[2];
    };

    struct Landmark 
    {
        static const unsigned int loc_left_eye = 0;
        static const unsigned int loc_right_eye = 1;
        static const unsigned int loc_nose = 2;
        static const unsigned int loc_left_lip = 3;
        static const unsigned int loc_right_lip = 4;
        static const unsigned int loc_x = 0;
        static const unsigned int loc_y = 1;
         
        Loc& operator[](int idx) { return locs[idx]; }

        Landmark(const int in_pts[5][2]) 
        {
            for (int i=0; i<lm_end; i++)
                locs[i] = in_pts[i];
        }

        Loc locs[5];
    };


    // Bookkeeping 
    static float CalcAreaGeneric(const int* x, const int* y, const std::size_t len)
    {
        // compute area, len has to >= 3
        if (len < 3)
            return 0.0f;

        float area = 0.0f;

        //i==0
        area += (x[0] * (y[1] - y[len - 1]));

        //i=1:len-1
        for (std::size_t i = 1; i < len-1; i++)
            area += (x[i] * (y[i + 1] - y[i - 1]));

        //i==len-1 -> won't work
        area += (x[len-1] * (y[0] - y[len-2]));

        return area/2.0f;
    }


