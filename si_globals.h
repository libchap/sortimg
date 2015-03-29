

// destination for the resized images: either absolute path, or relative to source dir
#define si_settings_dest_dir "."
#define si_settings_output_jpeg_quality 96

// how many images preload when walking through the source directory
#define si_settings_preload_images 3
// how many rescales pre-do when zoming with mouse wheel
#define si_settings_preload_zooms 3

// quantity parameters of the effect of mouse wheel zooming
#define si_settings_mousewheel_zoom_base 1.02
#define si_settings_mousewheel_step_base 40

// the target pixel size when marking the resize with keyboard key A (..F rescpectively)
#define si_settings_size_A 800
#define si_settings_size_B 1024
#define si_settings_size_C 1200
#define si_settings_size_D 1600
#define si_settings_size_E 1800
#define si_settings_size_F 1900

// initial ccords of the program window
#define si_settings_initial_width 1024
#define si_settings_initial_height 768
#define si_settings_initial_x 100
#define si_settings_initial_y 100


// some generally usable functions
extern QString size2string(const QSize & s);
extern bool string2size(const QString & s, QSize * r);
// in sortimg.cpp
