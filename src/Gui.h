#ifndef GUI_H
#define GUI_H

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "IconsFontAwesome5_c.h"

class Gui
{
public:
    Gui();

    void ApplyTheme();
    bool Initialize();
    bool PollEvent();
    void StartFrame();
    void EndFrame();
    void Destroy();

 //   static bool LoadTextureFromFile(const char *filename, GLuint *out_texture, int *out_width, int *out_height);


private:
     
};

namespace ImGui {
void LoadingIndicatorCircle(const char* label, const float indicator_radius,
                                   const ImVec4& main_color, const ImVec4& backdrop_color,
                                   const int circle_count, const float speed);
}

#endif // GUI_H
