/*

                    PROXYGEN

*/                                                                                       
// ImGui - standalone example application for SDL2 + OpenGL
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)
// (GL3W is a helper library to access OpenGL functions since there is no standard header to access modern OpenGL functions easily. Alternatives are GLEW, Glad, etc.)
#ifdef __APPLE__
#include "CoreFoundation/CoreFoundation.h"                  // fix path stuff
#endif


#include "imgui.h"
#include "imgui_impl_sdl_gl3.h"
#include <stdio.h>
#include "pstream.h"
#include <string>

#include <fstream>
#include <streambuf>

#include <iostream>
#include <stdlib.h>
#include <locale.h>
#include <stdint.h>
#include <thread>
#include <stdarg.h>
#include <stdbool.h>
#include <GL/gl3w.h>    // This example is using gl3w to access OpenGL functions (because it is small). You may use glew/glad/glLoadGen/etc. whatever already works for you.
#include <SDL.h>

//#include "keyrus.cpp"
#include "symon.cpp"

 #define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "nfd.h"
//#include <tchar.h>

#include "TextEditor.h"


         static int outWi =1280;
            static int outHe =720;
            static char outFont[64]="vcr.ttf";
            static char outWaterText[64]="WATERMARK";
            static char outWaterColor[12]="white";
            static float outWaterOpac=0.1f;
            static int outWaterSize =280;
            static int outWaterX =0;
            static int outWaterY =250;
            static int timecodebase = 24;
                 static int timecode[4] = { 01, 00,00, 00 };
 bool                ScrollToBottom;
    bool changesMade=false;
    bool LivePreviewIsOn=false;
    bool UpdatePreview=false;
 nfdchar_t *outPath = NULL;
nfdchar_t *path=""; 
 
float progress = 0.0f;
int width[10], height[10], bpp;
unsigned char* my_image_id[10];
unsigned int m_texture[10];
    char livepath[1024];
    char livethumbpath[512];
    char testframepath[1024]="";
    char exepath[512]="";
            char testthumbpath[512]="";
                      char ffplaytestframe[512]="";
                       char thumbpath[512]="";
                      
                      char const *folder = getenv("TMPDIR"); 

 std::thread t2;
        


 struct MyAppLog
{
    ImGuiTextBuffer     Buf;
    ImGuiTextFilter     Filter;
    ImVector<int>       LineOffsets;        // Index to lines offset
   

    void    Clear()     { Buf.clear(); LineOffsets.clear(); }
    void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
    {
        int old_size = Buf.size();
        va_list args;
        va_start(args, fmt);
        Buf.appendfv(fmt, args);
        va_end(args);
        for (int new_size = Buf.size(); old_size < new_size; old_size++)
            if (Buf[old_size] == '\n')
                LineOffsets.push_back(old_size);
        ScrollToBottom = true;
    }

    void    Draw(const char* title, bool* p_open = NULL)
    {
        ImGui::SetNextWindowSize(ImVec2(500,400), ImGuiCond_FirstUseEver);
        ImGui::Begin(title, p_open);
        if (ImGui::Button("Clear")) Clear();
        ImGui::SameLine();
        bool copy = ImGui::Button("Copy");
        ImGui::SameLine();
        Filter.Draw("Filter", -100.0f);
        ImGui::Separator();
        ImGui::BeginChild("scrolling", ImVec2(0,0), false, ImGuiWindowFlags_HorizontalScrollbar);
        if (copy) ImGui::LogToClipboard();

        if (Filter.IsActive())
        {
            const char* buf_begin = Buf.begin();
            const char* line = buf_begin;
            for (int line_no = 0; line != NULL; line_no++)
            {
                const char* line_end = (line_no < LineOffsets.Size) ? buf_begin + LineOffsets[line_no] : NULL;
                if (Filter.PassFilter(line, line_end))
                    ImGui::TextUnformatted(line, line_end);
                line = line_end && line_end[1] ? line_end + 1 : NULL;
            }
        }
        else
        {
            ImGui::TextUnformatted(Buf.begin());
        }

        if (ScrollToBottom)
            ImGui::SetScrollHere(1.0f);
        ScrollToBottom = false;
        ImGui::EndChild();
        ImGui::End();
    }
};

         static MyAppLog mylog;

void LoadImage(char* imagefile, int NumberImage) {
          if (width[1] != 0)  stbi_image_free(my_image_id[1]);
 
        my_image_id[NumberImage] = stbi_load(imagefile, &width[NumberImage], &height[NumberImage], &bpp, 3 );

        if (my_image_id[NumberImage] == NULL)
            {   

                    fprintf(stderr, "%s Failed to load texture\n",imagefile);
                    mylog.AddLog("[error] failed to load %s into memory \n",imagefile);
                    width[NumberImage]=0;
             }

       if (my_image_id[NumberImage] != NULL)
            {
                   // fprintf(stderr, "Loaded",imagefile);
                    //mylog.AddLog("NumberIMage=%d [info] loaded %s into memory \n",NumberImage,imagefile);
 
            }
            


        glGenTextures(1, &m_texture[NumberImage]);
        glBindTexture(GL_TEXTURE_2D, m_texture[NumberImage]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        if(bpp == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width[NumberImage], height[NumberImage], 0, GL_RGB, GL_UNSIGNED_BYTE, my_image_id[NumberImage]);
        else if(bpp == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width[NumberImage], height[NumberImage], 0, GL_RGBA, GL_UNSIGNED_BYTE, my_image_id[NumberImage]);

        glBindTexture(GL_TEXTURE_2D, 0);

        //stbi_image_free(myimage);
  
        // log.appendf("%s width=%d,height=%d,bpp=%d\n",imagefile,width,height,bpp);
        //fprintf(stderr,"%s width=%d,height=%d,bpp=%d\n",imagefile,width[NumberImage],height[NumberImage],bpp);

}



void LivePreview() {

     redi::ipstream proc2(testframepath, redi::pstreams::pstderr | redi::pstreams::pstderr);
/*
     // while (!proc2.rdbuf()->exited()) {
  //      mylog.AddLog("HERE\n");
   // }


   while (std::getline(proc2.err(), line))   { 
        mylog.AddLog("THERE\n");
    
          }


           while (std::getline(proc2.err(), line))   { 
        mylog.AddLog("THERE\n");*/

/*

    redi::ipstream proc2(testframepath, redi::pstreams::pstderr | redi::pstreams::pstderr) ;
  std::string line;
  // read child's stdout
 // mylog.AddLog("%s\n",testframepath);
  while (std::getline(proc2.out(), line)) {
      mylog.AddLog("[stderr1] %s\n",line.data());
  }
  // read child's stderr
  //while (std::getline(proc2.err(), NULL))   { 
  //  mylog.AddLog("[stderr2] %s\n",line.data());
    
   //       }
*/
        
    system(testframepath);
    UpdatePreview=true;  
    //if (proc2.eof()) UpdatePreview=true;   

   /*

          if (proc2.eof()) {
                mylog.AddLog("eof reached\n");
                UpdatePreview=true; 
                proc2.clear();
            }

            proc2.clear();

    */
            
}

void Logout() {
     system("if pgrep ffmpeg; then pkill ffmpeg; fi");
    testframepath[0] = 0;
     
    ffplaytestframe[0] = 0;
 mylog.AddLog("Hello from thread\n");
 
 mylog.AddLog("And other one Hello from thread\n");

   snprintf(testthumbpath,512,"%stestframe.mp4",folder);
            remove(testthumbpath);
                    
                    snprintf(testframepath,1024,"./ffmpeg -nostdin -loglevel info -y -ss `./ffmpeg -nostdin -i \"%s\" 2>&1 | grep Duration | awk '{print $2}' | tr -d , | awk -F ':' '{print ($3+$2*60+$1*3600)/2}' | cut -d',' -f1 ` -t 10 -i \"%s\"   -vf \"scale=%d:%d, drawtext=fontfile=%s: text='%s': fontcolor=%s@%.1f: fontsize=%d: x=%d: y=%d,drawtext=fontfile=vcr.ttf: timecode='%d\\:%d\\:%d\\:%d': r=%d: fontcolor=0xFFFFFF: fontsize=48: x=480: y=650,drawtext=fontfile=\"vcr.ttf\": text='': fontcolor=0xFFFFFF@0.5: fontsize=512: x=600: y=360\" -c:v libx264 -x264-params \"nal-hrd=cbr\" -pix_fmt yuv420p -b:v 2M \"%stestframe.mp4\"  ",outPath,outPath,outWi,outHe,outFont,outWaterText,outWaterColor,outWaterOpac,outWaterSize,outWaterX,outWaterY,timecode[0],timecode[1],timecode[2],timecode[3],timecodebase,folder);
                    printf("\n%s\n",testframepath);
                    mylog.AddLog("[info Logout] exec: %s\n",testframepath);
  redi::ipstream proc(testframepath, redi::pstreams::pstderr | redi::pstreams::pstderr);
  std::string word;
  std::string line;

  // read child's stdout
  while (!proc.rdbuf()->exited()) {
     proc >> word;
     
    // mylog.AddLog("[ffmpeg] %s\n",word.data());
     if (word =="frame=") mylog.AddLog("[ffmpeg] %s\n",word.data());
     if (word =="\0") mylog.AddLog("f\n");
   std::size_t found=word.find('frame');
  if (found!=std::string::npos) mylog.AddLog("FPUND\n");
    
     //if (word !="frame") mylog.AddLog("[ffmpeg] %s",word.data());
     
     
     //mylog.AddLog("GOTCHA GOTHCA")
     ScrollToBottom=true;
  }
  // read child's stderr
  while (std::getline(proc.out(), line))   { 
    mylog.AddLog("[stderr2] %s\n",line.data());
}

 mylog.AddLog("%s\n",testframepath);
     snprintf(ffplaytestframe,512,"./ffplay -noborder %s &",testthumbpath);
              redi::ipstream procFF(ffplaytestframe);

ScrollToBottom=false;
}

    bool show_proxygen_window = true;
        bool show_log_window = true;



int main(int argc, char *argv[])
{
    system("export TMPDIR=\"/tmp\"");
    system("echo $TMPDIR");
   locale_t nloc = newlocale(LC_NUMERIC_MASK,"POSIX",(locale_t) 0); 
    uselocale(nloc);
    // safe locale set 
char const *folder = getenv("TMPDIR");
if (folder == 0) 
        folder = "/tmp/";


// This makes relative paths work in C++ in Xcode by changing directory to the Resources folder inside the .app bundle
#ifdef __APPLE__    
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    char path[PATH_MAX];
    if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX)) // Error: expected unqualified-id before 'if'
    {
        // error!
    }
    CFRelease(resourcesURL); // error: expected constructor, destructor or type conversion before '(' token
using namespace std;
    chdir(path); // error: expected constructor, destructor or type conversion before '(' token
    //std::cout << "Current Path: " << path << std::endl; // error: expected constructor, destructor or type conversion before '<<' token
#endif



        static ImGuiTextBuffer log;
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Setup window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);
    SDL_Window* window = SDL_CreateWindow("proxygen 0.2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(1); // Enable vsync
    gl3wInit();

    // Setup ImGui binding
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    ImGui_ImplSdlGL3_Init(window);

    // Setup style
/* DARK THEME 
----
----------------------------------------------------------------------
*/
 
ImGuiStyle * style = &ImGui::GetStyle();
 
    style->WindowPadding = ImVec2(15, 15);
    style->WindowRounding = 5.0f;
    style->FramePadding = ImVec2(5, 5);
    style->FrameRounding = 4.0f;
    style->ItemSpacing = ImVec2(12, 8);
    style->ItemInnerSpacing = ImVec2(8, 6);
    style->IndentSpacing = 25.0f;
    style->ScrollbarSize = 15.0f;
    style->ScrollbarRounding = 9.0f;
    style->GrabMinSize = 5.0f;
    style->GrabRounding = 3.0f;
    style->FrameBorderSize = 0.0f;
 
    style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
    style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style->Colors[ImGuiCol_Border] = ImVec4(0.49f, 0.49f, 0.49f, 0.27f);
  
    style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
    style->Colors[ImGuiCol_FrameBg] = ImVec4(0.102f, 0.109f, 0.145f , 1.00f);
    style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_TitleBg] = ImVec4(0.102f, 0.109f, 0.145f , 1.00f);
    style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
    style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.102f, 0.109f, 0.145f , 1.00f);
    style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.102f, 0.109f, 0.145f , 1.00f);
    style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    //style->Colors[ImGuiCol_ComboBg] = ImVec4(0.19f, 0.18f, 0.21f, 1.00f);
    style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_Button] = ImVec4(0.102f, 0.109f, 0.145f , 1.00f);
    style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_Header] = ImVec4(0.102f, 0.109f, 0.145f , 1.00f);
    style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_Column] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_ColumnHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_ColumnActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    //style->Colors[ImGuiCol_CloseButton] = ImVec4(0.40f, 0.39f, 0.38f, 0.16f);
    //style->Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.40f, 0.39f, 0.38f, 0.39f);
    //style->Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.40f, 0.39f, 0.38f, 1.00f);
    style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
    style->Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
    ImGui::GetStyle().WindowBorderSize = 0.5f;
     style->Colors[ImGuiCol_Separator]              = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
     style->Colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.17f, 0.17f, 0.17f, 0.89f);
     style->Colors[ImGuiCol_SeparatorActive]        = ImVec4(0.17f, 0.17f, 0.17f, 0.89f);

 
/*

// -------- LIGHT THEME ----------

      ImVec4* colors = ImGui::GetStyle().Colors;
    ImGui::GetStyle().FramePadding = ImVec2(4.0f,2.0f);
    ImGui::GetStyle().ItemSpacing = ImVec2(8.0f,2.0f);
    ImGui::GetStyle().WindowRounding = 2.0f;
    ImGui::GetStyle().ChildRounding = 2.0f;
    ImGui::GetStyle().FrameRounding = 0.0f;
    ImGui::GetStyle().ScrollbarRounding = 0.0f;
    ImGui::GetStyle().GrabRounding = 1.0f;
    ImGui::GetStyle().WindowBorderSize = 1.0f;
    ImGui::GetStyle().FrameBorderSize = 1.0f;
    colors[ImGuiCol_Text]                   = ImVec4(0.00f, 0.00f, 0.00f, 0.85f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
    colors[ImGuiCol_Border]                 = ImVec4(0.00f, 0.00f, 0.00f, 0.44f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.64f, 0.65f, 0.66f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.64f, 0.65f, 0.66f, 0.40f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.71f, 0.70f, 0.70f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.43f, 0.43f, 0.43f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.63f, 0.63f, 0.63f, 0.78f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.43f, 0.44f, 0.46f, 0.78f);
    colors[ImGuiCol_Button]                 = ImVec4(0.61f, 0.61f, 0.62f, 0.40f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.57f, 0.57f, 0.57f, 0.52f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.61f, 0.63f, 0.64f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.64f, 0.64f, 0.65f, 0.31f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.58f, 0.58f, 0.59f, 0.55f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.52f, 0.52f, 0.52f, 0.55f);
    colors[ImGuiCol_Separator]              = ImVec4(0.56f, 0.56f, 0.56f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.17f, 0.17f, 0.17f, 0.89f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.17f, 0.17f, 0.17f, 0.89f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.80f, 0.80f, 0.80f, 0.56f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.39f, 0.39f, 0.40f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.39f, 0.39f, 0.40f, 0.67f);
    //colors[ImGuiCol_CloseButton]            = ImVec4(0.59f, 0.59f, 0.59f, 0.50f);
    //colors[ImGuiCol_CloseButtonHovered]     = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
    //colors[ImGuiCol_CloseButtonActive]      = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(0.56f, 0.56f, 0.56f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.71f, 0.72f, 0.73f, 0.57f);
    colors[ImGuiCol_ModalWindowDarkening]   = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(0.16f, 0.16f, 0.17f, 0.95f);

*/

   //ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them. 
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple. 
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'misc/fonts/README.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
   // io.Fonts->AddFontFromFileTTF("symonMedium.ttf", 14.0f,NULL,io.Fonts->GetGlyphRangesCyrillic());
 // ImFont* font = io.Fonts->AddFontFromFileTTF("./keyrusMedium.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
  // ImFont* font = io.Fonts->AddFontFromMemoryCompressedTTF(keyrusFont_compressed_data,keyrusFont_compressed_size,16.0f,NULL,io.Fonts->GetGlyphRangesCyrillic()); 
    
    ImFont* font = io.Fonts->AddFontFromMemoryCompressedTTF(symonFont_compressed_data,symonFont_compressed_size,13.0f,NULL,io.Fonts->GetGlyphRangesCyrillic()); 

  IM_ASSERT(font != NULL);
  

            

    bool show_moe_okno = false;
    bool show_demo_window = false;

    bool show_another_window = false;
    bool show_glitch_window = false;

    ImVec4 clear_color = ImVec4(0.102f, 0.109f, 0.145f, 1.00f);


     


   // VIDEO THUNB ffmpeg -itsoffset -1 -i rocket.mp4 -vframes 1 -filter:v scale="280:-1" thumb.jpg
      
    // Main loop
    bool done = false;
    while (!done)
    {


        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSdlGL3_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
        }
        ImGui_ImplSdlGL3_NewFrame(window);

        // 1. Show a simple window.
        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets automatically appears in a window called "Debug".
        {
          /*  static float f = 0.0f;
            static int counter = 0;
            ImGui::Text("Привет, world!");                           // Display some text (you can use a format string too)
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f    
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our windows open/close state
            ImGui::Checkbox("Another Window", &show_another_window);
            ImGui::Checkbox("Мое оккно", &show_moe_okno);

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (NB: most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        */

        }

        // 2. Show another simple window. In most cases you will use an explicit Begin/End pair to name your windows.
 
//ImGui::ShowTestWindow();
//ImGui::SetNextWindowBgAlpha(0.3f);
       // ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);





        if (show_log_window) { mylog.Draw("proxygen: log");}

          if (show_proxygen_window)
        {
            
            ImGui::Begin("proxygen 0.2", &show_another_window);

static float f = 0.0f;



            ImGui::Text("Hello from another window!");
           // ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f    
           // ImGui::ColorEdit3("clear color", (float*)&clear_color);
            ImGui::Columns(2, "mycolumns2", false);  // 3-ways, no border
              if (ImGui::Button("Browse")) {
                     

                      width[0] = 0;
                        width[1] = 0;
                
                      nfdresult_t result = NFD_OpenDialog("mp4;mkv;avi;mov", NULL, &outPath );
                      if ( result == NFD_OKAY )
                      {
                          
             
            printf("Path  : %s\n",  outPath );
            mylog.AddLog("[info] selected file: %s\n",outPath);
           
            snprintf(thumbpath,512,"%sthumb.jpg",folder);
            
            //if (remove(thumbpath)) {  mylog.AddLog("[info] removed %s\n",thumbpath);} 
            //else { mylog.AddLog("[error] having hard time removing: %s\n",thumbpath);}
             


             //generate thumbnail
            exepath[0]=0;
            snprintf(exepath,512,"ffmpeg -nostdin -loglevel panic -y -itsoffset -1 -ss `./ffmpeg -nostdin -i \"%s\" 2>&1 | grep Duration | awk '{print $2}' | tr -d , | awk -F ':' '{print ($3+$2*60+$1*3600)/2}' | cut -d',' -f1 ` -i \"%s\" -vframes 1 -filter:v scale=\"280:-1\"  $TMPDIR/thumb.jpg",outPath,outPath);
            
            printf("\n%s\n",exepath);
             mylog.AddLog("[info] execute: %s\n",exepath);
            system(exepath);


            // generate still fro LivePreview fullres
            exepath[0]=0;
             snprintf(exepath,512,"ffmpeg -nostdin -loglevel panic -y -itsoffset -1 -ss `./ffmpeg -nostdin -i \"%s\" 2>&1 | grep Duration | awk '{print $2}' | tr -d , | awk -F ':' '{print ($3+$2*60+$1*3600)/2}' | cut -d',' -f1 ` -i \"%s\" -vframes 1 $TMPDIR/live.jpg",outPath,outPath);
            
            printf("\n%s\n",exepath);
             mylog.AddLog("[info] execute: %s\n",exepath);
            system(exepath);
           

            snprintf(livepath,1024,"%stestframe.jpg",folder);
            snprintf(livethumbpath,512,"%slive.jpg",folder);
             mylog.AddLog("[livethumbpath] from Browse  %s  \n",livethumbpath);
            LoadImage(thumbpath,0);
            LivePreviewIsOn=true;
            //sprintf(mypath, "%s %s", mypath, path);

        //     strcpy(buffer2, path);
      //   strcpy(dirstring, path);
                 
       
 
    }
    else if ( result == NFD_CANCEL )
    {
        puts("User pressed cancel.");

    }
    else 
    {
        printf("Error: %s\n", NFD_GetError() );
            mylog.AddLog("[error] error in NFD_DIALOG\n");
    }

                
                }


  if (ImGui::Button("Clear")) {
                    if (outPath != NULL) {

                        outPath = NULL;
                        width[0] = 0;
                        width[1] = 0;
                        mylog.AddLog("[info] Closed the file\n");

                    }
                     else {
                        
                     }
                 }

                 if (ImGui::Button("Show in Finder")) {
                    if (outPath != NULL) {
 
                    }
                     else {
                        
                     }
                 }






  if (ImGui::Button("Play")) {
                    if (outPath != NULL) {
                            char playinput[512];
                        snprintf(playinput,512,"./ffplay -noborder \"%s\"",outPath);
                        mylog.AddLog("[info] exec:%s\n",playinput);
                  system(playinput);
                  

                    }
                     else {
                         ImGui::OpenPopup("Warning");
                  
                     }
                 }
                  
                 

      ImGui::NextColumn();
             
                 if ((width[0] > 0) && (width[1] > 0)) {
  

            ImVec2 pos = ImGui::GetCursorScreenPos();
             // printf("Batman width=%d,height=%d,bpp=%d",width,height,bpp);
             ImGui::Image((void *)m_texture[1], ImVec2(width[0],height[0]), ImVec2(0,0), ImVec2(1,1), ImColor(255,255,255,255), ImColor(255,255,255,128));


              if (ImGui::IsItemClicked()) {       if (outPath != NULL) {
                            char playinput[512];
                        snprintf(playinput,512,"./ffplay -noborder -exitonmousedown \"%stestframe.jpg\"",folder);     ///very bad code
                        mylog.AddLog("[info] exec:%s\n",playinput);
                  system(playinput);
                  

                    }
                     else {
                         ImGui::OpenPopup("Warning");
                  
                     }     }
                if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                float region_sz = 32.0f;
                float region_x = io.MousePos.x - pos.x - region_sz * 0.5f; if (region_x < 0.0f) region_x = 0.0f; else if (region_x > width[0] - region_sz) region_x = width[0] - region_sz;
                float region_y = io.MousePos.y - pos.y - region_sz * 0.5f; if (region_y < 0.0f) region_y = 0.0f; else if (region_y > height[0] - region_sz) region_y = height[0] - region_sz;
                float zoom = 4.0f;
                ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
                ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
                ImVec2 uv0 = ImVec2((region_x) / width[0], (region_y) / height[0]);
                ImVec2 uv1 = ImVec2((region_x + region_sz) / width[0], (region_y + region_sz) /height[0]);
                ImGui::Image((void *)m_texture[1], ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, ImColor(255,255,255,255), ImColor(255,255,255,128));
                ImGui::EndTooltip();
            }
            
           }
            ImGui::Columns(1);
             ImGui::Text("\n");
            ImGui::Separator();
              

            if (outPath != NULL) { ImGui::Text("Filename: "); ImGui::SameLine();ImGui::Text(outPath); }
            
   
            ImGui::Columns(2, "mycolumns3", false); 

            ImGui::InputInt("Out Width", &outWi);  
            ImGui::InputInt("Out Height", &outHe);

            ImGui::InputText("Font", outFont,64);
             ImGui::Button("Browse font");  ImGui::SameLine(); ImGui::Button("Reset font");      
           
                      ImGui::InputInt("Timecode base", &timecodebase);
              //ImGui::InputInt("Watermark Y", &outWaterY); 
ImGui::LabelText("Timecode start", ""); 
                     
                      ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.05f);
                ImGui::DragInt("hh",&timecode[0],1,0,255);ImGui::SameLine();
                ImGui::DragInt("mm",&timecode[1],1,0,59);ImGui::SameLine();
                   ImGui::DragInt("ss",&timecode[2],1,0,59);ImGui::SameLine();
                      ImGui::DragInt("frms",&timecode[3],1,0,24); 
      ImGui::PopItemWidth(); 

                   
            
            ImGui::NextColumn();
            if ( ImGui::InputText("Watermark", outWaterText,64) ) { 
                                LivePreviewIsOn=true;
                          

                      }
            ImGui::InputText("Watermark Color", outWaterColor,12);
           // ImGui::InputFloat("Watermark Opacity",&outWaterOpac,0.1f,0,1.0f);  
            if ( ImGui::SliderFloat("Watermark Opacity", &outWaterOpac, 0.0f, 1.0f, "opacity = %.1f") ) {

  LivePreviewIsOn=true;

            }

             
             if ( ImGui::DragInt("Watermark SIze",&outWaterSize,1,0,455)


            )   LivePreviewIsOn=true;
           // ImGui::InputInt("Watermark X", &outWaterX);
             if (ImGui::SliderInt("Watermark X", &outWaterX, 0, outWi))  LivePreviewIsOn=true;

                if (ImGui::SliderInt("Watermark Y", &outWaterY, 0, outHe))  LivePreviewIsOn=true;  
            ImGui::InputInt("Width", &width[0]);
            

ImGui::Columns(1);

/* static char commandline[256]="s"; ImGui::InputText("RUN:", commandline,256);
  if (ImGui::Button("Test Out")) {
                     redi::ipstream proc(commandline, redi::pstreams::pstderr | redi::pstreams::pstderr);
  std::string line;
  // read child's stdout
  while (std::getline(proc.out(), line)) {
     mylog.AddLog("[stderr1] %s\n",line.data());
  }
  // read child's stderr
  while (std::getline(proc.err(), line))   { 
    mylog.AddLog("[stderr2] %s\n",line.data());
}
}
*/
    
                


        //ImGui::Text(" \n"); 
        ImGui::Separator();        
                if (ImGui::Button("Test Frame")) {

                      if (width[0] == 0) {


                ImGui::OpenPopup("Warning");
            
   }
   else 
   {

/*
                      //char testthumbpath[512]="";
                      //char thumbpath[512]="";
                      char ffplaytestframe[512]="";
                      char testframepath[1024]="";
                      char const *folder = getenv("TMPDIR"); 
            snprintf(testthumbpath,512,"%stestframe.jpg",folder);
            remove(testthumbpath);
                           
                    snprintf(testframepath,1024,"./ffmpeg  -nostdin  -loglevel panic -y -ss `./ffmpeg -i \"%s\" 2>&1 | grep Duration | awk '{print $2}' | tr -d , | awk -F ':' '{print ($3+$2*60+$1*3600)/2}' | cut -d',' -f1 ` -i \"%s\" -vframes 1 -vf \"scale=%d:%d, drawtext=fontfile=%s: text='%s': fontcolor=%s@%.1f: fontsize=%d: x=%d: y=%d,drawtext=fontfile=vcr.ttf: timecode='%d\\:%d\\:%d\\:%d': r=25: fontcolor=0xFFFFFF: fontsize=48: x=480: y=650,drawtext=fontfile=\"vcr.ttf\": text='': fontcolor=0xFFFFFF@0.5: fontsize=512: x=600: y=360\"  \"%stestframe.jpg\"",outPath,outPath,outWi,outHe,outFont,outWaterText,outWaterColor,outWaterOpac,outWaterSize,outWaterX,outWaterY,timecode[0],timecode[1],timecode[2],timecode[3],folder);
                    snprintf(livepath,1024,"%stestframe.jpg",folder);
                    printf("\n%s\n",testframepath);
                    mylog.AddLog("Test frame [info] exec: %s\n",testframepath);
  
                    system(testframepath);
                          LoadImage(livepath,1);   
                 // snprintf(ffplaytestframe,512,"./ffplay %s",testthumbpath);
                  //system(ffplaytestframe);

*/

    LivePreviewIsOn=true;
            } 
              }
              ImGui::SameLine();
/*

                if (width[1]>0) {



  ImVec2 pos = ImGui::GetCursorScreenPos();
                    ImGui::Image((void *)m_texture[1], ImVec2(width[0], height[0]), ImVec2(0,0), ImVec2(1,1), ImColor(255,255,255,255), ImColor(255,255,255,128));
                  if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                float region_sz = 32.0f;
                float region_x = io.MousePos.x - pos.x - region_sz * 0.5f; if (region_x < 0.0f) region_x = 0.0f; else if (region_x > width[0] - region_sz) region_x = width[0] - region_sz;
                float region_y = io.MousePos.y - pos.y - region_sz * 0.5f; if (region_y < 0.0f) region_y = 0.0f; else if (region_y > height[0] - region_sz) region_y = height[0] - region_sz;
                float zoom = 4.0f;
                ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
                ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
                ImVec2 uv0 = ImVec2((region_x) / width[0], (region_y) / height[0]);
                ImVec2 uv1 = ImVec2((region_x + region_sz) / width[0], (region_y + region_sz) /height[0]);
                ImGui::Image((void *)m_texture[1], ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, ImColor(255,255,255,255), ImColor(255,255,255,128));
                ImGui::EndTooltip();
            }

}
/*}

                 if (ImGui::Button("Test 10secs")) {

                      if (width == 0) {


                ImGui::OpenPopup("Warning");
            
   }
   else 
   {


              
            snprintf(testthumbpath,512,"%stestframe.mp4",folder);
            remove(testthumbpath);
                    
                    snprintf(testframepath,1024,"ffmpeg -nostdin -loglevel info -y -ss `ffmpeg -i \"%s\" 2>&1 | grep Duration | awk '{print $2}' | tr -d , | awk -F ':' '{print ($3+$2*60+$1*3600)/2}' | cut -d',' -f1 ` -t 10 -i \"%s\"   -vf \"scale=%d:%d, drawtext=fontfile=%s: text='%s': fontcolor=%s@%.1f: fontsize=%d: x=%d: y=%d,drawtext=fontfile=vcr.ttf: timecode='%d\\:%d\\:%d\\:%d': r=%d: fontcolor=0xFFFFFF: fontsize=48: x=480: y=650,drawtext=fontfile=\"vcr.ttf\": text='': fontcolor=0xFFFFFF@0.5: fontsize=512: x=600: y=360\" -c:v libx264 -x264-params \"nal-hrd=cbr\" -pix_fmt yuv420p -b:v 2M \"%stestframe.mp4\"",outPath,outPath,outWi,outHe,outFont,outWaterText,outWaterColor,outWaterOpac,outWaterSize,outWaterX,outWaterY,timecode[0],timecode[1],timecode[2],timecode[3],timecodebase,folder);
                    printf("\n%s\n",testframepath);
                    mylog.AddLog("[info] exec: %s\n",testframepath);
                   // system(testframepath);
                          redi::ipstream proc(testframepath, redi::pstreams::pstderr | redi::pstreams::pstderr);
  std::string line;
  // read child's stdout
  while (std::getline(proc.out(), line)) {
     mylog.AddLog("[stderr2] %s\n",line.data());
    
 
   
  }
  // read child's stderr
  while (std::getline(proc.err(), line))   { 
    mylog.AddLog("[stderr] %s\n",line.data());
}


                  snprintf(ffplaytestframe,512,"ffplay %s &",testthumbpath);
                  system(ffplaytestframe);


            } 
              }
              */



  if (ImGui::Button("Test 10 sec")) {   
         if (width[0] == 0) {


                ImGui::OpenPopup("Warning");
            
   }
   else 
   {
       std::thread t1(Logout);
 t1.detach();
}

  }
              ImGui::SameLine();
                 if (ImGui::Button("Reset Values")) {

                        outWi =1280;
              outHe =720;
              snprintf(outFont,64,"vcr.ttf");
               snprintf(outWaterText,64,"WATERMARK");
                snprintf(outWaterColor,12,"white");
              outWaterOpac=0.1f;
              outWaterSize =280;
              outWaterX =0;
              outWaterY =250;
              timecode[0] = 1;
timecode[1] = 0;
              timecode[2] = 0;
              timecode[3] = 0;
              timecodebase=24;


        }

          

         
            

         
//ImGui::Text("Asdad\n");
 // ImGui::Separator();


  if (ImGui::BeginPopupModal("Warning", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("No file to process. \nTo select a file press Browse\n\n");
                      
                ImGui::Separator();
                //static int dummy_i = 0;
                //ImGui::Combo("Combo", &dummy_i, "Delete\0Delete harder\0");

              

                if (ImGui::Button("OK", ImVec2(120,0))) { ImGui::CloseCurrentPopup(); mylog.AddLog("[info] warning popup\n"); }
                ImGui::SetItemDefaultFocus();
              
                ImGui::EndPopup();
            }
  

            if (ImGui::Button("Close Me")){
                done = true;
                show_proxygen_window = false;
            }




            if (LivePreviewIsOn) {


                    char testthumbpath[512]="";
                      char thumbpath[512]="";
                 //     livethumbpath[0]=0;
                  //     snprintf(livethumbpath,512,"%slive.jpg",folder);

                       snprintf(testframepath,1024,"ffmpeg -nostdin -loglevel info -y -i \"%s\" -vframes 1 -vf \"scale=%d:%d, drawtext=fontfile=%s: text='%s': fontcolor=%s@%.1f: fontsize=%d: x=%d: y=%d,drawtext=fontfile=vcr.ttf: timecode='%d\\:%d\\:%d\\:%d': r=25: fontcolor=0xFFFFFF: fontsize=48: x=480: y=650,drawtext=fontfile=\"vcr.ttf\": text='': fontcolor=0xFFFFFF@0.5: fontsize=512: x=600: y=360\"  \"%stestframe.jpg\"  ",livethumbpath,outWi,outHe,outFont,outWaterText,outWaterColor,outWaterOpac,outWaterSize,outWaterX,outWaterY,timecode[0],timecode[1],timecode[2],timecode[3],folder);
                    snprintf(thumbpath,1024,"%stestframe.jpg",folder);
                    //printf("\n%s\n",testframepath);
                    // to much stuff 
                    //mylog.AddLog("[info] exec: %s\n",testframepath);
                
                     mylog.AddLog("[livethumbpath] from LivePreview %s  \n",livethumbpath);
                     std::thread t2(LivePreview);
                     t2.join();
                    mylog.AddLog("Here in Live, Running system");  
                     
                    
                   //system(testframepath);    
                    system("echo FUCK");
                    
                    mylog.AddLog("\n and it was:%s\n",testframepath);
                          LivePreviewIsOn=false;




            }

            if (UpdatePreview) {  LoadImage(livepath,1);UpdatePreview=false; }



            ImGui::End();
        }

        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);
            ImGui::Text("Hello from another window!");
            


            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }
        if (show_moe_okno)
        {
            ImGui::Begin("Moe okno",&show_moe_okno);
            //ImGuiIO& io = ImGui::GetIO();
          /*  ImGui::Text("batman");
            if (width > 0) {


            ImVec2 pos = ImGui::GetCursorScreenPos();
             // printf("Batman width=%d,height=%d,bpp=%d",width,height,bpp);
             ImGui::Image((void *)m_texture, ImVec2(width, height), ImVec2(0,0), ImVec2(1,1), ImColor(255,255,255,255), ImColor(255,255,255,128));
                if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                float region_sz = 32.0f;
                float region_x = io.MousePos.x - pos.x - region_sz * 0.5f; if (region_x < 0.0f) region_x = 0.0f; else if (region_x > width - region_sz) region_x = width - region_sz;
                float region_y = io.MousePos.y - pos.y - region_sz * 0.5f; if (region_y < 0.0f) region_y = 0.0f; else if (region_y > height - region_sz) region_y = height - region_sz;
                float zoom = 4.0f;
                ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
                ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
                ImVec2 uv0 = ImVec2((region_x) / width, (region_y) / height);
                ImVec2 uv1 = ImVec2((region_x + region_sz) / width, (region_y + region_sz) /height);
                ImGui::Image((void *)m_texture, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, ImColor(255,255,255,255), ImColor(255,255,255,128));
                ImGui::EndTooltip();
            }
            
           }

*/


 static char buf2[64] = "batman.jpg"; ImGui::InputText("a", buf2, 64);  
   if (ImGui::Button("Load")) { LoadImage(buf2,0);}

   
   if (ImGui::Button("Up")) { progress += 0.1f;}
   ImGui::ProgressBar(progress, ImVec2(0.0f,0.0f));

            ImGui::TextWrapped("And now some textured buttons..");
            ImGui::Text("wo333w");
      static char buf1[64] = "script/proxygen.sh"; ImGui::InputText("default", buf1, 64);    
    // ImGui::InputInt("input int width", &width);
    if (ImGui::Button("Clear")) { log.clear();}  
    ImGui::SameLine();


        if (ImGui::Button("Clear")) { log.clear();}  
    ImGui::SameLine();
     if (ImGui::Button("Button")) {

         log.appendf("Path relative to the working directory is: %s\n", argv[0]);

             char line[1024];
    
    FILE *pipe;
    
    /* Get a pipe where the output from the scripts comes in */
    pipe = popen(buf1, "r");
    if (pipe == NULL) {  /* check for errors */
        perror(argv[0]); /* report error message */
        return 1;        /* return with exit code indicating error */
    }

    /* Read script output from the pipe line by line */
  
    while (fgets(line, 1024, pipe) != NULL) {
        /*printf("Script output line %d: %s", linenr, line);
        puts(line);*/

        
        log.appendf("%s",line);
    }
    log.appendf("\n");
 
    
    /* Once here, out of the loop, the script has ended. */
    pclose(pipe); /* Close the pipe */
              }
 ImGui::Text("Buffer contents:  %d bytes", log.size());

            ImGui::BeginChild("Log");
           ImGui::TextUnformatted(log.begin(), log.end());
              

             ImGui::EndChild();
          
              ImTextureID my_tex_id = io.Fonts->TexID; 
            float my_tex_w = (float)io.Fonts->TexWidth;
            float my_tex_h = (float)io.Fonts->TexHeight;

            ImGui::Text("%.0fx%.0f", my_tex_w, my_tex_h);
          
            //ImGui::Image(my_tex_id, ImVec2(my_tex_w, my_tex_h), ImVec2(0,0), ImVec2(1,1), ImColor(255,255,255,255), ImColor(255,255,255,128));
         
            ImGui::TextWrapped("And now some textured buttons..");
            



            if (ImGui::Button("Close Me"))
                show_moe_okno= false;
           
            ImGui::End();
        }
            
            
            // 3. Show the ImGui demo window. Most of the sample code is in ImGui::ShowDemoWindow(). Read its code to learn more about Dear ImGui!
        if (show_demo_window)
        {
            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver); // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        // Rendering
        glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        ImGui_ImplSdlGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Cleanup

    ImGui_ImplSdlGL3_Shutdown();
    ImGui::DestroyContext();
      stbi_image_free(my_image_id[0]);
stbi_image_free(my_image_id[1]);
    

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    system("if pgrep ffmpeg; then pkill ffmpeg; fi");

    return 0;
}
