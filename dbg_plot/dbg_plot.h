#ifndef __DBG_PLOT_H__
#define __DBG_PLOT_H__

#include <string>
#include <map>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <iterator>
#include <array>
#include <utility>

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#include <direct.h>
#endif

struct Naming
{
    static std::string ReplaceCharacter(const std::string& in, const char orig, const char to_replace)
    {
        std::string ret = in;
        for (std::size_t i = 0; i < in.length(); i++)
        {
            if (ret[i] == orig)
                ret[i] = to_replace;
        }
        return ret;
    }

    static std::string ConvertToValidVarName(const std::string& in)
    {
        std::string ret = in;
        ret = ReplaceCharacter(ret, ' ', '_');
        ret = ReplaceCharacter(ret, '-', '_');
        ret = ReplaceCharacter(ret, '[', '_');
        ret = ReplaceCharacter(ret, ']', '_');
        ret = ReplaceCharacter(ret, ')', '_');
        ret = ReplaceCharacter(ret, '(', '_');
        ret = ReplaceCharacter(ret, '.', '_');
        return ret;
    }
};

namespace DbgPlot
{
    using Loc = std::pair<double, double>;
    using Idx = std::size_t;
    using Idx2 = std::pair<Idx, Idx>;

    template <typename T>
    struct Rectangle : std::array<T, 4>
    {
        template <typename ...Args>
        Rectangle(Args&&... args) : std::array<T, 4>{ {std::forward<Args>(args)...} } {}
        inline T x() const { return this->at(0); }
        inline T y() const { return this->at(1); }
        inline T w() const { return this->at(2); }
        inline T h() const { return this->at(3); }
        inline T x0() const { return x(); }
        inline T x1() const { return x() + w(); }
        inline T y0() const { return y(); }
        inline T y1() const { return y() + h(); }
    };

    const std::string option_none("None");
    const std::string imfile_none("None");
    const std::size_t array_elem_num_per_line(16);
    const std::string default_py_folder_name("./pyfigs/");
    const std::pair<double, double> lim_none({ 0, 0 });
    const Rectangle<int> rect_empty{ 0, 0, 0, 0 };

    template <typename T>
    struct Buf1d : std::vector<T>
    {
        template <typename ...Args>
        Buf1d(Args&&... args) : std::vector<T>(std::forward<Args>(args)...) {}

        void StreamOut(std::ofstream& ofs, const std::string& array_name) const
        {
            if (this->size() == 0)
                return;

            ofs << array_name << " = [";

            for (std::size_t idx=0; idx<this->size()-1; idx++)
            {
                ofs << this->at(idx) << ", ";
                // add end of line and indent every array_elem_num_per_line (predefined 16) elements
                if (idx % array_elem_num_per_line == 0 && idx!=0)
                    ofs << std::endl << "    "; 
            }
            // last element does not neeed ","
            ofs << this->at(this->size()-1) << "]" << std::endl;
        }
    };

    template <typename T>
    struct Buf2d : std::vector<std::vector<T>>
    {
        void Resize(const int width, const int height)
        {
            this->resize(height);
            for (auto i = this->begin(); i != this->end(); i++)
                i->resize(width);
        }

        void StreamOut(std::ofstream& ofs, const std::string& img_name) const
        {
            ofs << img_name << "= [";
            std::size_t height = this->size();
            std::size_t width = this->at(0).size();

            for (std::size_t j = 0; j < height; j++)
            {
                ofs << "[";
                for (std::size_t i = 0; i < width; i++)
                {
                    ofs << this->at(j).at(i);
                    if (i != (width - 1))
                         ofs << ",";
                }
                ofs << "]";
                if (j != (height - 1))
                    ofs << ",";
                ofs << std::endl;
            }
            ofs << "]" << std::endl;
        }
    };

    template <typename T>
    struct Plot1d : public std::vector<
        std::tuple< Buf1d<T>, std::string >
    >
    {
        void StreamOut(std::ofstream& ofs, const std::string& fig_name, const std::string& axes_name) const
        {
            for (std::size_t i = 0; i < this->size(); i++)
            {
                auto name_y = "array_" + std::to_string(i) + "_y";
                auto array_y = std::get<0>(this->at(i));

                array_y.StreamOut(ofs, name_y);

                ofs << axes_name << ".plot( " << name_y;

                auto opt_str = std::get<1>(this->at(i));
                if (opt_str == option_none)
                    ofs << ")" << std::endl;
                else
                    ofs << ",\"" << opt_str << "\")" << std::endl;

                ofs << std::endl;
            }

        }
    };
    
    template <typename T>
    struct Plot2d : public std::vector <
        std::tuple < Buf1d<T>, Buf1d<T>, std::string >
    >
    {
        void StreamOut(std::ofstream& ofs, const std::string& fig_name, const std::string& axes_name) const
        {
            const std::string& array_name("array");

            for (std::size_t i = 0; i<this->size(); i++)
            {

                auto array_x = std::get<0>(this->at(i));
                auto array_y = std::get<1>(this->at(i));
                auto opt_str = std::get<2>(this->at(i));

                auto name_y = array_name + "_" + std::to_string(i) + "_y";
                auto name_x = array_name + "_" + std::to_string(i) + "_x";

                array_x.StreamOut(ofs, name_x);
                array_y.StreamOut(ofs, name_y);

                ofs << axes_name << ".plot(" << name_x << "," << name_y;

                if (opt_str == option_none)
                    ofs << ")" << std::endl;
                else
                    ofs << ",\"" << opt_str << "\")" << std::endl;

                ofs << std::endl;
            }
        }
    };

    class ImShowFromFile
    {
    public:
        ImShowFromFile()
            : str_imfilepath(imfile_none),
              str_option(option_none),
              enabled(false),
              xywh(rect_empty)
        {}

    public:
        void SetFilePath(
            const std::string& in_file_path, 
            const Rectangle<int>& in_xywh = rect_empty)
        {
            enabled = true;
            str_imfilepath = in_file_path;
            xywh = in_xywh;
        }

        void SetOption(const std::string& in_option)
        {
            str_option = in_option;
        }

        void StreamOut(std::ofstream& ofs, const std::string& fig_name, const std::string& axes_name) const
        {
            if (enabled && str_imfilepath != imfile_none)
            {
                const std::string orig_im_name = fig_name + "_" + axes_name + "_im";
                const std::string im_name = MakeImgName(orig_im_name);

                ofs << im_name << " = np.array(Image.open(\""
                    << str_imfilepath
                    << "\"), np.intc)" << std::endl;

                if (xywh == rect_empty)
                {
                    StreamOutImshow(ofs, axes_name, im_name);
                }
                else
                {
                    int x = xywh.x();
                    int y = xywh.y();
                    int w = xywh.w();
                    int h = xywh.h();

                    const std::string im_name_rect = im_name + "_rect";
                    ofs << im_name_rect << " = " << im_name << "["
                        << y << ":" << y + h << "," 
                        << x << ":" << x + w << "] " << std::endl;

                    StreamOutImshow(ofs, axes_name, im_name_rect);
                }
            }
        }
    private:
        void StreamOutImshow(std::ofstream& ofs, const std::string& axes_name, const std::string& img_name) const
        {
            if (str_option == option_none)
                ofs << axes_name << ".imshow(" << img_name << ")" << std::endl;
            else
                ofs << axes_name << ".imshow(" << img_name << ", " << str_option << ")" << std::endl;
        }

        std::string MakeImgName(const std::string& in) const
        {
            std::string ret = in;
            for (auto i = ret.begin(); i != ret.end(); i++)
            {
                if (*i == ' ' || *i == '[' || *i == ']')
                    *i = '_';
            }
            return ret;
        }

    private:
        std::string str_imfilepath;
        std::string str_option;
        bool enabled;
        Rectangle<int> xywh;
    };

    template <typename T>
    class ImShowToPy
    {
    public:
        ImShowToPy()
            : enabled(false)
        {
        }

        template <typename Tin>
        void CopyData(
            const std::string& im_name,
            const Tin* buf,
            const int width,
            const int height,
            const int stride,
            const std::string& format,
            const Rectangle<int> region)
        {
            enabled = true;
            
            name = im_name;


            img.Resize(region.w(), region.h());

            for (int j = 0; j < region.h(); j++)
            {
                for (int i = 0; i < region.w(); i++)
                {
                    // assuming typecasting is safe
                    int x = region.x() + i;
                    int y = region.y() + j;
                    img[j][i] = static_cast<T>(buf[y * stride + x]);
                }
            }
        }

        void StreamOut(std::ofstream& ofs, const std::string& fig_name, const std::string& axes_name) const
        {
            if (!enabled)
                return;

            std::string img_name = MakeImgName(fig_name + "_im_" + name);
            img.StreamOut(ofs, img_name);

            ofs << axes_name << ".imshow(" << img_name << ", " << "cmap=\"gray\"" << ")" << std::endl;
        }

    private:
        //TODO: refactor this (repeated fxn)
        std::string MakeImgName(const std::string& in) const
        {
            std::string ret = in;
            for (auto i = ret.begin(); i != ret.end(); i++)
            {
                if (*i == ' ' || *i == '[' || *i == ']')
                    *i = '_';
            }
            return ret;
        }

    private:
        std::string name;
        Buf2d<T> img;
        bool enabled;
    };

    class Patch : std::vector<std::tuple<Rectangle<int>, std::string>>
    {
    public:
        void Add(const Rectangle<int>& in_rect, const std::string& in_opt)
        {
            emplace_back(std::make_tuple(in_rect, in_opt));
        }

        void StreamOut(std::ofstream& ofs, const std::string& fig_name, const std::string& axes_name) const
        {
            for (const_iterator i = cbegin(); i != cend(); i++)
            {
                const Rectangle<int>& r = std::get<0>(*i);

                int x = r.x();
                int y = r.y(); 
                int w = r.w(); 
                int h = r.h(); 
                ofs << axes_name << ".add_patch(patches.Rectangle("
                    << "[" << x << "," << y << "], "
                    << w << "," << h;

                const std::string& opt = std::get<1>(*i);
                if (opt == option_none)
                {
                    ofs << " ))" << std::endl;
                }
                else
                {
                    ofs << "," << opt << "))" << std::endl;
                }
            }
        }

    };

    class Text : public std::vector< std::tuple<Loc, std::string, std::string> >
    {
    public:
        void Add(
            const Loc& in_xy, 
            const std::string& in_content, 
            const std::string& in_opt)
        {
            emplace_back(std::make_tuple(in_xy, in_content, in_opt));
        }

        void StreamOut(std::ofstream& ofs, const std::string& fig_name, const std::string& axes_name) const
        {
            for (auto i = this->cbegin(); i != this->cend(); i++)
            {
                auto x = std::get<0>(*i).first;
                auto y = std::get<0>(*i).second;
                const std::string& content = std::get<1>(*i);
                const std::string& opt = std::get<2>(*i);

                ofs << axes_name << ".text(" << x << "," << y << ","
                    << "\"" << content << "\"";

                if (opt == option_none)
                    ofs << ")" << std::endl;
                else
                    ofs << "," << opt << ")" << std::endl;
            }
        }
    };

    class Axes
    {
    public:
        Axes()
            : str_title(option_none),
              str_xlabel(option_none),
              str_ylabel(option_none),
              lim_x(lim_none),
              lim_y(lim_none)
        {}
    public:
        template <typename T>
        void plot(
            const T* value,
            const std::size_t len,
            const std::string& str = option_none)
        {
            plotter1d.emplace_back(
                std::make_tuple(
                    Buf1d<double>(&value[0], &value[len]),
                    str
                ));
        }

        template <typename T1, typename T2>
        void plot(
            const T1* value_t1,
            const T2* value_t2,
            const std::size_t len,
            const std::string& str = option_none)
        {
            plotter2d.emplace_back(
                std::make_tuple(
                    Buf1d<double>(&value_t1[0], &value_t1[len]),
                    Buf1d<double>(&value_t2[0], &value_t2[len]),
                    str));
        }

        void set_title(const std::string& in_str) { str_title = in_str; }
        void set_xlabel(const std::string& in_str) { str_xlabel = in_str; }
        void set_ylabel(const std::string& in_str) { str_ylabel = in_str; }
        void set_xlim(const double& left, const double& right) { lim_x = { left, right }; }
        void set_ylim(const double& top, const double& bottom) { lim_y = { top, bottom }; }

        void imshow(const std::string& file_path, const std::string& str_option=option_none)
        {
            imshow_file.SetFilePath(file_path);
            imshow_file.SetOption(str_option);
        }

        void imshow(const std::string& file_path,
            const int ix,
            const int iy,
            const int iw,
            const int ih,
            const std::string& str_option = option_none)
        {
            imshow_file.SetFilePath(file_path, { ix, iy, iw, ih });
            imshow_file.SetOption(str_option);
        }

        template <typename T>
        void imshow(
            const std::string& im_name,
            const T* buf,
            const int width,
            const int height,
            const int stride,
            const std::string& format,
            const Rectangle<int> region = rect_empty)
        {
            // support gray only for now
            if (format != "gray")
                return;

            const Rectangle<int> region_whole{ 0, 0, width, height };

            const Rectangle<int>* p_reg;
            if (region == rect_empty)
                p_reg = &region_whole;
            else
                p_reg = &region;

            imshow_py.CopyData(
                im_name,
                buf, width, height, stride,
                format,
                *p_reg);
        }

        void add_patch(
            const Rectangle<int>& in_rect, 
            const std::string& in_option=option_none)
        {
            patcher.Add(in_rect, in_option);
        }

        void text(
            const Loc& in_xy,
            const std::string& in_text,
            const std::string& in_option = option_none)
        {
            texter.Add(in_xy, in_text, in_option);
        }

    public:
        void StreamOut(std::ofstream& ofs, const std::string& fig_name, const std::string& axes_name) const
        {
            plotter1d.StreamOut(ofs, fig_name, axes_name);

            plotter2d.StreamOut(ofs, fig_name, axes_name);

            imshow_file.StreamOut(ofs, fig_name, axes_name);

            imshow_py.StreamOut(ofs, fig_name, axes_name);

            patcher.StreamOut(ofs, fig_name, axes_name);

            texter.StreamOut(ofs, fig_name, axes_name);

            if (str_title != option_none)
                ofs << axes_name << ".set_title(\"" << str_title << "\")" << std::endl;
            if (str_xlabel != option_none)
                ofs << axes_name << ".set_xlabel(\"" << str_xlabel << "\")" << std::endl;
            if (str_ylabel != option_none)
                ofs << axes_name << ".set_ylabel(\"" << str_ylabel << "\")" << std::endl;
            if (lim_x != lim_none)
                ofs << axes_name << ".set_xlim(" << lim_x.first << "," << lim_x.second << ")" << std::endl;
            if (lim_y != lim_none)
                ofs << axes_name << ".set_ylim(" << lim_y.first << "," << lim_y.second << ")" << std::endl;
        }

    private:
        Plot1d<double> plotter1d;
        Plot2d<double> plotter2d;
        ImShowFromFile imshow_file;
        ImShowToPy<double> imshow_py;
        Patch patcher;
        Text texter;
        std::string str_title;
        std::string str_xlabel;
        std::string str_ylabel;
        std::pair<double, double> lim_x;
        std::pair<double, double> lim_y;
    };

    class Figure : public std::vector<Axes>
    {
    public:
        Figure(const std::string& in_name) : 
            name(in_name),
            cols(1), 
            rows(1),
            str_suptitle(option_none)
        {}

        // one dimentional subplots
        void subplots(const Idx in_nums = 1)
        {
            if (in_nums == 1)
            {
                cols = 1;
                rows = 1;
            }
            else
            {
                cols = in_nums;
                rows = 1;
            }
            resize(cols * rows);
        }

        // two dimenstional subplots
        void subplots(const Idx in_rows, const Idx in_cols)
        {
            cols = in_cols;
            rows = in_rows;
            resize(cols * rows);
        }

        void suptitle(const std::string& in_str, const std::string& opt = option_none)
        {
            str_suptitle = in_str;
        }

        Axes& operator[](const Idx pos) 
        { 
            if (pos >= size())
            {
                resize(pos+1);
                // User didn't explicitly assign # of rows & columns, before calling [] operator.
                // However, we don't want to through exception, since the purpose of debug module is debugging, 
                // handling error of it is too much trouble.
                // So, in this case, we reset (cols, rows) to (1, size())

                // TODO: output some warning message in such case
                cols = size();
                rows = 1;
            }
            return at(pos);
        }

        Axes& operator[](const Idx2& rc)
        {
            if (rc.first >= rows || rc.second >= cols)
            {
                rows = rc.first;
                cols = rc.second;
                resize(rows * cols);
            }
            return at(SubToIdx(rc));
        }
    private:
        Idx2 IdxToSub(const Idx& in_idx) const
        {
            return std::make_pair(
                static_cast<int>(std::ceil(in_idx / cols)),
                static_cast<int>(in_idx % cols));
        }

        Idx SubToIdx(const Idx2& in_sub) const
        {
            return (in_sub.first * cols + in_sub.second);
        }

    public:
        void StreamOut(const std::string& out_folder, const std::string& fig_name, std::ofstream& ofs) const
        {
            ofs << "import matplotlib.pyplot as plt" << std::endl
                << "import matplotlib.patches as patches" << std::endl
                << "import matplotlib.image as mpimg" << std::endl
                << "import numpy as np" << std::endl
                << "from PIL import Image" << std::endl;
            
            ofs << std::endl;

            ofs << "fig, axes = plt.subplots(" << rows << "," << cols << ")" << std::endl;

            if (cols == 1 && rows == 1)
            {
                // when we do
                // > fig, axes = plt.subplots(1,1) 
                // we cannot say fig[0] or axes[0] since the statement returns scalar (instead of array)
                // so we do 
                // > fig = [fig]
                // > axes = [axes]
                // to force the fig and axes become array
                ofs << "fig = [fig]" << std::endl
                    << "axes = [axes]" << std::endl;
            }

            
            for (Idx axes_idx = 0; axes_idx < size(); axes_idx++)
            {
                std::string axes_name;
                if (rows != 1 && cols != 1)
                {
                    // 2D array
                    Idx2 sub = IdxToSub(axes_idx);

                    axes_name = "axes[" + std::to_string(sub.first) + "]" + "[" + std::to_string(sub.second) + "]";
                }
                else
                {
                    // 1D array
                    axes_name = "axes[" + std::to_string(axes_idx) + "]";
                }

                at(axes_idx).StreamOut(ofs, fig_name, axes_name);
            }

            ofs << std::endl;

            if (str_suptitle != option_none)
                ofs << "fig.suptitle(\"" << str_suptitle << "\")" << std::endl;

            ofs << std::endl
                << "plt.ion()" << std::endl
                << "plt.draw()" << std::endl
                << "manager = plt.get_current_fig_manager()" << std::endl
                << "manager.full_screen_toggle()" << std::endl
                << "plt.show(block=False)" << std::endl
                << "plt.pause(0.001)" << std::endl;
                //<< "input(\"Press[enter] to continue.\")" << std::endl;
                if (cols == 1 && rows == 1)
                    ofs << "fig[0].savefig(\"";
                else
                    ofs << "fig.savefig(\"";
                ofs << out_folder + fig_name << ".png\")" << std::endl;
                ofs << "plt.close(\"all\")" << std::endl;
        }


    private:
        std::string name;
        Idx cols;
        Idx rows;
        std::string str_suptitle;
    };

    class DbgPlot : public std::map<std::string, Figure>
    {
    public:
        DbgPlot(const std::string& in_folder = default_py_folder_name)
            : folder_name(Naming::ReplaceCharacter(in_folder, '\\', '/'))
        {
        }

    public:
        Figure& operator[](const std::string& in_fig_name)
        {
            const std::string fig_name = Naming::ConvertToValidVarName(in_fig_name);
            try
            {
                return at(fig_name);
            }
            catch (const std::out_of_range& oor)
            {
                auto a = oor.what(); a = a;
                emplace(fig_name, Figure(fig_name));
                return at(fig_name);
            }
        }

        ~DbgPlot()
        {
            stream_out();
        }

    private:
        void stream_out()
        {
            MkDir(folder_name);
            
            for (const_iterator i = cbegin(); i != cend(); i++)
            {
                std::string fig_file = Naming::ReplaceCharacter(i->first, ' ', '_');
                std::string fig_file_path = folder_name + "/" + fig_file + ".py";
                {
                    std::ofstream ofs(fig_file_path);
                    i->second.StreamOut(folder_name, fig_file, ofs);
                }
            }
        }

        std::string folder_name;

        void MkDir(const std::string& dir)
        {
#if defined(_WIN32) || defined(WIN64)
            // Recursively create directory - thx, StackOverflow
            std::size_t pos = 0;

            do
            {
                pos = dir.find_first_of("/", pos + 1);
                // support ASCII version only
                if (CreateDirectoryA(dir.substr(0, pos).c_str(), NULL) != 0 &&
                    ERROR_ALREADY_EXISTS != GetLastError())
                {
                    std::cout << "Unknown error creating directory" << dir << std::endl;
                    break;
                }
            } while (pos != std::string::npos);
#endif
        }

    };

    template <typename T>
    struct DbgBase : std::map<std::string, T>
    {
        DbgBase(const std::string& in_folder_name)
            : folder_name(in_folder_name)
        {
        }

        T& operator[](const std::string& fig_name)
        {
            try
            {
                return this->at(fig_name);
            }
            catch (const std::out_of_range& oor)
            {
                this->emplace(fig_name, T());
                auto a = oor.what();
                a = a;
                return this->at(fig_name);
            }
        }

        void StreamOut()
        {
            MkDir(folder_name);

            for (auto i = this->cbegin(); i != this->cend(); i++)
            {
                const std::string py_file_name = Naming::ConvertToValidVarName(i->first);
                const std::string py_file_path = folder_name + "/" + py_file_name + ".py";

                std::ofstream ofs(py_file_path);

                i->second.stream_out(folder_name, py_file_name, ofs);
            }
        }

        ~DbgBase()
        {
            StreamOut();
        }

        void MkDir(const std::string& dir)
        {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
            // Recursively create directory - thx, StackOverflow
            std::size_t pos = 0;

            do
            {
                pos = dir.find_first_of("/", pos + 1);
                // support ASCII version only
                if (CreateDirectoryA(dir.substr(0, pos).c_str(), NULL) != 0 &&
                    ERROR_ALREADY_EXISTS != GetLastError())
                {
                    std::cout << "Unknown error creating directory" << dir << std::endl;
                    break;
                }
            } while (pos != std::string::npos);
#endif
        }

        std::string folder_name;
    };

    struct ListText : std::vector<
        std::pair<std::string, Buf1d<double>> >
    {
        template <typename T>
        void add(const std::string& desc, const T* in_arr=nullptr, const std::size_t len=0)
        {
            emplace_back(
                std::make_pair(
                    Naming::ConvertToValidVarName(desc),
                    Buf1d<double>( in_arr, &in_arr[len] )
                )
            );
        }

        void comment(const std::string& desc)
        {
            emplace_back(
                std::make_pair(
                    desc,
                    Buf1d<double>(0)
                )
            );
        }
        void stream_out (
            const std::string& folder_name,
            const std::string& py_file_name,
            std::ofstream& ofs) const
        {
            for (auto i = cbegin(); i != cend(); i++)
            {
                // if add without any array, meaning comments in the py code
                if (i->second.size() == 0)
                    ofs << "#" << i->first << std::endl;
                else
                    i->second.StreamOut(ofs, i->first);
            }
        }
    };

    using DbgListText = DbgBase<ListText>;
};

#endif //__DBG_PLOT_H__

