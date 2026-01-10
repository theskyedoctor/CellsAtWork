/* warning this program uses 100% CPU because it is using all available clock cycles to check for input.
 * Don't run multiple oh god.
 * This will be fixed after I learn how to!
*/

#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include "SDL3_image/SDL_image.h"
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

/* Class Prototypes */
class LTexture
{
public:
    //initializes texture variables
    LTexture();

    //cleans up texture variables
    ///These two functions are actually referred to as the constructor and deconstructor.
    ///Basically, whenever this class is initialized, LTexture() runs, and whenever it is destroyed ~LTexture() runs.
    ///this goes for every texture we use, so hopefully you can see the use case. behind only declaring this once.
    ~LTexture();

    //Load texture from disk
    bool loadFromFile( std::string path );

    //Cleans up texture
    ///this will be called by the deconstructor, and we could get away with not having it seperate,
    ///but it is good for organization
    void destroy();

    //draw texture
    void render( float x, float y );

    //Gets texture attributes
    ///we could store these as public variables, and not have to access them through functions,
    ///but that is not optimal for larger projects
    int getWidth();
    int getHeight();
    bool isLoaded();

private:
    //contains texture data
    ///This is a pointer to some SDL internal thing. No clue what it does, but we don't really need to know.
    SDL_Texture* mTexture;

    //Texture dimensions
    int mWidth;
    int mHeight;
};

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

//The renderer used to draw to the window
///There are also tools called "surfaces" that use the CPU to render.
///this tool uses the gpu, and is therefore far more performant
SDL_Renderer* gRenderer{ nullptr };

//the PNG image we will be rendering
///you could likely load as many textures as you wanted this way.
///seems inefficient and I bet that there is a way to load in lots of textures into an atlas of some kind.
LTexture gPngTexture, gBmpTexture;

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

            //The currently rendered texture
            LTexture* currentTexture = &gPngTexture;
            SDL_Color bgColor{ 0xff, 0xff, 0xff, 0xff };

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
                    //on keyboard key press
                    else if ( e.type == SDL_EVENT_KEY_UP )
                    {
                        if ( e.key.key == SDLK_SPACE)
                        {
                            if ( currentTexture == &gPngTexture )
                            {
                                currentTexture = &gBmpTexture;
                            }
                            else
                            {
                                currentTexture = &gPngTexture;
                            }
                        }
                    }
                }
                //reset background color to white
                bgColor.r = 0xFF;
                bgColor.g = 0xFF;
                bgColor.b = 0xFF;

                //set background color based on key state
                /// there are more traditional ways to get keycodes, but keyboard state is the best for movement
                /// otherwise you could use the code above
                const bool* keyStates = SDL_GetKeyboardState( nullptr );
                if ( keyStates[ SDL_SCANCODE_SPACE ] )
                {
                    bgColor.r = 0xFF;
                    bgColor.g = 0xB7;
                    bgColor.b = 0xCE;


                }

                //Fill the background
                ///could maybe input opengl shaders that you make in shader toy here? might be worth trying
                SDL_SetRenderDrawColor( gRenderer, bgColor.r, bgColor.g, bgColor.b, 0xFF );
                SDL_RenderClear( gRenderer );

                //render image on screen
                /// we write it as 0.f instead of 0.0 because the graphics card uses a weird notation.
                /// if I teach you some openGL youll use it a lot. otherwise dont worry too much about it
                currentTexture->render( ( kScreenWidth - currentTexture->getWidth() ) / 2.f, ( kScreenHeight - currentTexture->getHeight() ) / 2.f );

                //update screen
                SDL_RenderPresent( gRenderer );
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

/* Class implementations */
//LTexture implementation
/// we could also declare these in the prototype, but that can cause weird bugs
LTexture::LTexture():
    //initialize texture variables
    mTexture{ nullptr },
    mWidth{ 0 },
    mHeight{ 0 }
{

}

LTexture::~LTexture()
{
    //clean up texture
    destroy();
}

bool LTexture::loadFromFile( std::string path )
{
    //clean up texture if it already exists
    destroy();

    //load surface
    ///fucky if statement
    ///also looks like this code loads the image into a CPU surface, then converts it to a GPU texture.
    ///It looks like we do this so the cpu can modify the texture before it is rendered.
    ///might be a good way to render the game?
    if ( SDL_Surface* loadedSurface = IMG_Load( path.c_str() ); loadedSurface == nullptr )
    {
        SDL_Log( "Unable to load image %s! SDL_image error: %s\n", path.c_str(), SDL_GetError() );
    }
    else
    {
        //create texture from surface
        ///apparently in modern c++ you can declare a variable and check a condition on the same line. cool?
        ///personally im never gonna use it, but the tutorial did so i guess its okay.
        if ( mTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface ); mTexture == nullptr )
        {
            SDL_Log( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
        }
        else
        {
            //get image dimensions
            mWidth = loadedSurface->w;
            mHeight = loadedSurface->h;
        }

        //clean up loaded surface
        SDL_DestroySurface( loadedSurface );
    }

    //Return success if texture loaded
    return mTexture != nullptr;
}

void LTexture::destroy()
{
    //clean up texture
    SDL_DestroyTexture( mTexture );
    mTexture = nullptr;
    mWidth = 0;
    mHeight = 0;
}

void LTexture::render( float x, float y )
{
    //set texture position
    ///SDL_FRect defines where on the screen we are gonna draw it.
    ///we also convert the width and height to float to make sure no weird bugs happen.
    ///static_cast is just the best way to do that in modern c++
    SDL_FRect dstRect{ x, y, static_cast<float>( mWidth ), static_cast<float>( mHeight ) };

    //render texture
    SDL_RenderTexture( gRenderer, mTexture, nullptr, &dstRect);
}

///simple variable accessors.
///we use these cause having public variables is cringe.
int LTexture::getWidth()
{
    return mWidth;
}

int LTexture::getHeight()
{
    return mHeight;
}

bool LTexture::isLoaded()
{
    return mTexture != nullptr;
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
        //create window with renderer
        if ( SDL_CreateWindowAndRenderer( "SDL3 example: Eukariot", kScreenWidth, kScreenHeight, 0, &gWindow, &gRenderer ) == false )
        {
            SDL_Log("Window could not be created! SDL_Error: %s\n", SDL_GetError() );
            success = false;
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
    ///im pondering how this will work for ascii still.
    ///dwarf fortress actually uses an older version of SDL so its clearly possible.
    if ( gPngTexture.loadFromFile( "../assets/eukariot.png" ) == false )
    {
        SDL_Log("SDL could not load image!\n");
        success = false;
    }
    if ( gBmpTexture.loadFromFile( "../assets/SDL-logo.bmp" ) == false )
    {
        SDL_Log("SDL could not load image!\n");
        success = false;
    }

    return success;
}

void close()
{
    //clean up surface
    gPngTexture.destroy();

    //destroy window
    SDL_DestroyRenderer( gRenderer );
    gRenderer = nullptr;
    SDL_DestroyWindow( gWindow );
    gWindow = nullptr;

    //quit sdl subsystems
    SDL_Quit();
}