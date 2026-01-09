/* warning this program uses 100% CPU because it is using all available clock cycles to check for input.
 * Don't run multiple oh god.
 * This will be fixed after I learn how to!
*/

#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include <string>

/* Constants */
//Screen Dimension Constants
/*Apparently dynamic scaling windows would be a bad idea for a whole bunch of reasons. (fractional scaling and general instability)
I think that's why older games had a dropdown menu for a bunch of different resolutions.
We could probably do dynamic scaling for this project,
but if we were making something huge like Helldivers it might be a bad idea.
also I think some modern games detect the monitor's size once and set it that way. also an option.*/
constexpr int kScreenWidth{ 1280 };
constexpr int kScreenHeight{ 640 };

/* Function Prototypes */
//Starts up SDL and creates window
bool init();

//loads media
bool loadMedia();

//frees media and shuts down SDL
void close();

/* Global Variables */
//The window we'll be rendering to
SDL_Window* gWindow{ nullptr };

//The surface contained by the window
SDL_Surface* gScreenSurface{ nullptr };

//The image we will load and show on the screen
SDL_Surface* gHelloWorld{ nullptr };

int main()
{
    //Final exit code
    int exitCode{ 0 };

    //initialize
    if ( init() == false )
    {
        SDL_Log( "Unable to initialize program!\n" );
        exitCode = 1;
    }
    else
    {
        //load media
        if ( loadMedia() == false )
        {
            SDL_Log( "Unable to load media!\n" );
            exitCode = 2;
        }
        else
        {
            //the quit flag
            bool quit{ false };

            //The event data
            ///You can use events to get mouse input and keypresses.
            SDL_Event e;
            SDL_zero( e );

            //The main loop
            while ( quit == false)
            {
                //Get event data
                ///this event checks if the user closes the window, and then ends the loop allowing the program to end.
                while ( SDL_PollEvent( &e ) == true )
                {
                    //If event is quit type
                    if ( e.type == SDL_EVENT_QUIT )
                    {
                        quit = true;
                    }
                }

                //File the surface white
                ///could maybe input opengl shaders that you make in shader toy here? might be worth trying
                SDL_FillSurfaceRect( gScreenSurface, nullptr, SDL_MapSurfaceRGB( gScreenSurface, 0xFF, 0xFF, 0xFF ) );

                //render image on screen
                SDL_BlitSurface( gHelloWorld, nullptr, gScreenSurface, nullptr );

                //update the surface
                SDL_UpdateWindowSurface( gWindow );
            }
        }
    }

    //Clean up
    /// on modern c++ you can aparantly automate this process, but it pretty heavily impacts performance.
    /// That probably doesnt matter on such a small scale, but it might be smart to save performance where we cane
    /// in case we fuck up somewhere else that is.
    close();

    return exitCode;
}

/* Function Implementations*/

bool init()
{
    //initialization flag
    bool success{ true };

    //initialize SDL
    if ( SDL_Init( SDL_INIT_VIDEO ) == false )
    {
        SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        success = false;
    }
    else
    {
        //create window
        ///apparently in modern c++ you can declare a variable and check a condition on the same line. cool?
        ///personally im never gonna use it, but the tutorial did so i guess its okay.
        if ( gWindow = SDL_CreateWindow( "SDL3 example: Hello SDL3", kScreenWidth, kScreenHeight, 0 ); gWindow == nullptr )
        {
            SDL_Log("Window could not be created! SDL_Error: %s\n", SDL_GetError() );
            success = false;
        }
        else
        {
            //get window surface
            gScreenSurface = SDL_GetWindowSurface( gWindow );
        }
    }
    return success;
}

bool loadMedia()
{
    //File loading flag
    bool success{ true };

    //Load splash image
    ///there are like a billion different ways to load images in SDL3, this is just one option.
    ///textures for example are hardware accelerated, and are probably the main way of having images.
    ///im pondering how this will work for ascii still.
    ///dwarf fortress actually uses an older version of SDL so its clearly possible.
    std::string imagePath{ "../assets/eukariot.png" };
    ///another cursed if statement
    if ( gHelloWorld = SDL_LoadPNG( imagePath.c_str() ); gHelloWorld == nullptr )
    {
        SDL_Log("SDL could not load image! SDL_Error: %s\n", SDL_GetError() );
        success = false;
    }

    return success;
}

void close()
{
    //clean up surface
    SDL_DestroySurface( gHelloWorld );
    gHelloWorld = nullptr;

    //destroy window
    SDL_DestroyWindow( gWindow );
    gWindow = nullptr;
    gScreenSurface = nullptr;

    //quit sdl subsystems
    SDL_Quit();
}