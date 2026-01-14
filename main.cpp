/* warning this program uses 100% CPU because it is using all available clock cycles to check for input.
 * Don't run multiple oh god.
 * This will be fixed after I learn how to!
*/

#include <ostream>

#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include "SDL3_image/SDL_image.h"
#include "SDL3_ttf/SDL_ttf.h"
#include <string>
#include <sstream>

/* Constants */
//Screen Dimension Constants
/*Apparently dynamic scaling windows would be a bad idea for a whole bunch of reasons. (fractional scaling and general instability)
I think that's why older games had a dropdown menu for a bunch of different resolutions.
We could probably do dynamic scaling for this project,
but if we were making something huge like Helldivers it might be a bad idea.
also I think some modern games detect the monitor's size once and set it that way. also an option.*/
constexpr int kScreenWidth{ 1280 };
constexpr int kScreenHeight{ 640 };
constexpr int kScreenFps{ 60 };

/* Class Prototypes */
class LTexture
{
public:
    //Symbolic constant
    static constexpr float kOriginalSize = -1.f;

    //initializes texture variables
    LTexture();

    //cleans up texture variables
    ///These two functions are actually referred to as the constructor and deconstructor.
    ///Basically, whenever this class is initialized, LTexture() runs, and whenever it is destroyed ~LTexture() runs.
    ///this goes for every texture we use, so hopefully you can see the use case. behind only declaring this once.
    ~LTexture();

    //Load texture from disk
    bool loadFromFile( std::string path, Uint8 r, Uint8 g, Uint8 b );

#if defined(SDL_TTF_MAJOR_VERSION)
    //creates texture from text
    bool loadFromRenderedText( std::string textureText, SDL_Color textColor );
#endif

    //Cleans up texture
    ///this will be called by the deconstructor, and we could get away with not having it seperate,
    ///but it is good for organization
    void destroy();

    //Sets color modulation
    void setColor( Uint8 r, Uint8 g, Uint8 b );

    //set opacity
    void setAlpha( Uint8 alpha );

    //set blend mode
    void setBlending( SDL_BlendMode blendMode );

    //draw texture
    void render( float x, float y, SDL_FRect* clip = nullptr, float width = kOriginalSize, float height = kOriginalSize, double degrees = 0.0, SDL_FPoint* center = nullptr, SDL_FlipMode flipMode = SDL_FLIP_NONE );

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

/* planned render overhaul classes
class Transform2D
{

};

class SpriteDef
{

};

class GameObject
{

};*/

class LButton
{
public:
    //Button dimensions
    static constexpr int kButtonWidth = 300;
    static constexpr int kButtonHeight = 200;

    //initalizes internal variables
    LButton();

    //Sets top left potiion
    void setPosition( float x, float y );

    //handles mouse event
    void handleEvent( SDL_Event* e );

    //show button sprite
    void render();

private:
    enum class eButtonSprite
    {
        MouseOut = 0,
        MouseOverMotion = 1,
        MouseDown = 2,
        MouseUp = 3
    };

    //top left position
    SDL_FPoint mPosition;

    //currently used global sprite
    eButtonSprite mCurrentSprite;
};

class LTimer
{
public:
    //initialize variables
    LTimer();

    //The various clock actions
    void start();
    void stop();
    void pause();
    void unpause();

    //get the times time
    Uint64 getTicksNS();

    bool isStarted();
    bool isPaused();

private:
    //the clock time when the timer started
    Uint64 mStartTicks;

    //The ticks stored when the timer was paused
    Uint64 mPausedTicks;

    bool mPaused;
    bool mStarted;
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

//global font
TTF_Font* gFont{ nullptr };

//the PNG image we will be rendering
///you could likely load as many textures as you wanted this way.
///seems inefficient and I bet that there is a way to load in lots of textures into an atlas of some kind.
LTexture gPngTexture, gBmpTexture, gSpriteSheetTexture, gTextTexture, gButtonSpriteTexture, gTimeTextTexture, gFpsTexture;

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

            //set color constants
            constexpr int kColorMagnitudeCount = 3;
            constexpr Uint8 kColorMagnitudes[ kColorMagnitudeCount ] = { 0x00, 0x7F, 0xFF };
            enum class eColorChannel
            {
                TextureRed = 0,
                TextureGreen = 1,
                TextureBlue = 2,
                TextureAlpha = 3,

                BackgroundRed = 4,
                BackgroundGreen = 5,
                BackgroundBlue = 6,

                Total = 7,
                Unknown = 8
            };

            //initialize colors
            Uint8 colorChannelsIndices[ static_cast<Uint8>( eColorChannel::Total ) ];
            colorChannelsIndices[ static_cast<int>( eColorChannel::TextureRed ) ] = 2;
            colorChannelsIndices[ static_cast<int>( eColorChannel::TextureGreen ) ] = 2;
            colorChannelsIndices[ static_cast<int>( eColorChannel::TextureBlue ) ] = 2;
            colorChannelsIndices[ static_cast<int>( eColorChannel::TextureAlpha ) ] = 2;

            colorChannelsIndices[ static_cast<int>( eColorChannel::BackgroundRed ) ] = 2;
            colorChannelsIndices[ static_cast<int>( eColorChannel::BackgroundGreen ) ] = 2;
            colorChannelsIndices[ static_cast<int>( eColorChannel::BackgroundBlue ) ] = 2;

            //Initialize blending
            gPngTexture.setBlending( SDL_BLENDMODE_BLEND );
            gBmpTexture.setBlending( SDL_BLENDMODE_BLEND );

            constexpr int kButtonCount = 1;
            LButton buttons[ kButtonCount ];
            buttons[0].setPosition(0, 0);

            //application timer
            LTimer timer;

            //in memory text stream
            std::stringstream timeText;

            bool vsyncEnabled{ true };

            //FPS cap toggle
            bool fpsCapEnabled{ false };

            //timer to cap frame rate
            LTimer capTimer;

            //Time spent rendering
            Uint64 renderingNS{ 0 };

            std::stringstream frameTimeText;

            //The main loop
            while ( quit == false)
            {
                //start frame time
                capTimer.start();

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
                    /*else if ( e.type == SDL_EVENT_KEY_UP )
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
                    }*/
                    //Reset start time on return keypress
                    else if( e.type == SDL_EVENT_KEY_DOWN )
                    {
                        //Start/stop
                        if( e.key.key == SDLK_RETURN )
                        {
                            if( timer.isStarted() )
                            {
                                timer.stop();
                            }
                            else
                            {
                                timer.start();
                            }
                        }
                        //Pause/unpause
                        else if( e.key.key == SDLK_SPACE )
                        {
                            if( timer.isPaused() )
                            {
                                timer.unpause();
                            }
                            else
                            {
                                timer.pause();
                            }
                        }
                    }
                    //Handle button events
                    for( int i = 0; i < kButtonCount; ++i )
                    {
                        buttons[ i ].handleEvent( &e );
                    }
                }


                //set background color based on key state
                /// there are more traditional ways to get keycodes, but keyboard state is the best for movement
                /// otherwise you could use the code above
                eColorChannel channelToUpdate = eColorChannel::Unknown;
                const bool* keyStates = SDL_GetKeyboardState( nullptr );
                if ( keyStates[ SDL_SCANCODE_SPACE ] )
                {
                    channelToUpdate = eColorChannel::TextureAlpha;
                }

                if ( channelToUpdate != eColorChannel::Unknown )
                {
                    //Cycle through channel values
                    colorChannelsIndices[ static_cast<int>( channelToUpdate ) ]++;
                    if ( colorChannelsIndices[ static_cast<int>( channelToUpdate ) ] >= kColorMagnitudeCount )
                    {
                        colorChannelsIndices[ static_cast<int>( channelToUpdate ) ] = 0;
                    }
                }
                //update timer text
                if ( timer.isStarted() == true )
                {
                    timeText.str( "" );
                    timeText << "milliseconds since start time " << ( timer.getTicksNS() / 1000000 );
                    SDL_Color textColor{ 0x00, 0x00, 0x00, 0xFF};
                    gTimeTextTexture.loadFromRenderedText( timeText.str(), textColor );
                }

                //update framerate timer text
                if ( renderingNS != 0 )
                {
                    double framesPerSecond{ 1000000000.0 / static_cast<double>( renderingNS ) };

                    frameTimeText.str( "" );
                    frameTimeText << "FPS: " << ( vsyncEnabled ? "(VSYNC )" : "" ) << ( fpsCapEnabled ? "(Cap) " : "" ) <<framesPerSecond;
                    SDL_Color textColor{ 0x00, 0x00, 0x00, 0xFF };
                    gFpsTexture.loadFromRenderedText( frameTimeText.str(), textColor );
                }


                //Fill the background
                ///could maybe input opengl shaders that you make in shader toy here? might be worth trying
                SDL_SetRenderDrawColor( gRenderer,
                    kColorMagnitudes[ colorChannelsIndices[ static_cast<int>( eColorChannel::BackgroundRed ) ] ],
                    kColorMagnitudes[ colorChannelsIndices[ static_cast<int>( eColorChannel::BackgroundGreen ) ] ],
                    kColorMagnitudes[ colorChannelsIndices[ static_cast<int>( eColorChannel::BackgroundBlue ) ] ],
                    0xFF );
                SDL_RenderClear( gRenderer );

                //initialize sprite clip
                constexpr float kSpriteSize = 100.f;
                SDL_FRect spriteClip{ 0.f, 0.f, kSpriteSize, kSpriteSize };

                //initialize sprite size
                SDL_FRect spriteSize{ 0.f, 0.f, kSpriteSize, kSpriteSize};

                //use top left sprite
                spriteClip.x = 0.f;
                spriteClip.y = 0.f;

                //set sprite size to original size
                spriteSize.w = kSpriteSize;
                spriteSize.h = kSpriteSize;

                for( int i = 0; i < kButtonCount; i++ )
                {
                    buttons[ i ].render();
                }

                //draw original sized sprite
                //gSpriteSheetTexture.render( 0.f, 0.f, &spriteClip, spriteSize.w, spriteSize.h, 45 );

                //draw text
                gTimeTextTexture.render( ( kScreenWidth - gTimeTextTexture.getWidth() ) / 2.f, ( kScreenHeight - gTimeTextTexture.getHeight() ) );

                //draw fps
                gFpsTexture.render( ( kScreenWidth - gFpsTexture.getWidth() ), ( 0 ) );

                //render text
                //gTextTexture.render( ( kScreenWidth - gTextTexture.getWidth() ) / 2.f, (kScreenHeight - gTextTexture.getHeight() ) );

                //render image on screen
                /// we write it as 0.f instead of 0.0 because the graphics card uses a weird notation.
                /// if I teach you some openGL youll use it a lot. otherwise dont worry too much about it
                currentTexture->render( ( kScreenWidth - currentTexture->getWidth() ) / 2.f, ( kScreenHeight - currentTexture->getHeight() ) / 2.f );

                //update screen
                SDL_RenderPresent( gRenderer );

                //get time to render frame
                renderingNS = capTimer.getTicksNS();

                constexpr Uint64 nsPerFrame = 1000000000 / kScreenFps;
                if ( fpsCapEnabled && renderingNS < nsPerFrame )
                {
                    //sleep remaining frame time
                    Uint64 sleepTime = nsPerFrame - renderingNS;
                    SDL_DelayNS( sleepTime );

                    //get frame time including sleep time
                    renderingNS = capTimer.getTicksNS();
                }
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

bool LTexture::loadFromFile( std::string path, Uint8 r, Uint8 g, Uint8 b )
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
        if ( SDL_SetSurfaceColorKey( loadedSurface, true, SDL_MapSurfaceRGB( loadedSurface, r, g, b) ) == false)
        {
            SDL_Log( "Unable to color key! SDL error: %s", SDL_GetError() );
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
        }

        //clean up loaded surface
        SDL_DestroySurface( loadedSurface );
    }

    //Return success if texture loaded
    return mTexture != nullptr;
}

#if defined(SDL_TTF_MAJOR_VERSION)
bool LTexture::loadFromRenderedText(std::string textureText, SDL_Color textColor)
{
    //Clean up existing texture
    destroy();

    //load text surface
    if ( SDL_Surface* textSurface = TTF_RenderText_Blended( gFont, textureText.c_str(), 0, textColor ); textSurface == nullptr )
    {
        SDL_Log( "Unable to render text surface! SDL_ttf Error: %s\n", SDL_GetError() );
    }
    else
    {
        //create texture from surface
        if ( mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface ); mTexture == nullptr )
        {
            SDL_Log( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
        }
        else
        {
            mWidth = textSurface->w;
            mHeight = textSurface->h;
        }
        //Free temp surface
        SDL_DestroySurface( textSurface );
    }

    //Return success if texture loaded
    return mTexture != nullptr;
}
#endif

void LTexture::destroy()
{
    //clean up texture
    SDL_DestroyTexture( mTexture );
    mTexture = nullptr;
    mWidth = 0;
    mHeight = 0;
}

void LTexture::setColor(Uint8 r, Uint8 g, Uint8 b)
{
    SDL_SetTextureColorMod( mTexture, r, g, b );
}

void LTexture::setAlpha(Uint8 alpha)
{
    SDL_SetTextureAlphaMod( mTexture, alpha );
}

void LTexture::setBlending(SDL_BlendMode blendMode)
{
    SDL_SetTextureBlendMode( mTexture, blendMode );
}

void LTexture::render( float x, float y, SDL_FRect* clip, float width, float height, double degrees, SDL_FPoint* center, SDL_FlipMode flipMode )
{
    //set texture position
    ///SDL_FRect defines where on the screen we are gonna draw it.
    ///we also convert the width and height to float to make sure no weird bugs happen.
    ///static_cast is just the best way to do that in modern c++
    SDL_FRect dstRect{ x, y, static_cast<float>( mWidth ), static_cast<float>( mHeight ) };

    //Default to clip dimensions if clip is given
    if ( clip != nullptr )
    {
        dstRect.w = clip->w;
        dstRect.h = clip->h;
    }

    //Resize if new dimensions are given
    if ( width > 0 )
    {
        dstRect.w = width;
    }
    if ( height > 0 )
    {
        dstRect.h = height;
    }

    //render texture
    SDL_RenderTextureRotated( gRenderer, mTexture, clip, &dstRect, degrees, center, flipMode);
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

//LButton Implementation
LButton::LButton():
    mPosition{ 0.f, 0.f },
    mCurrentSprite{ eButtonSprite::MouseOut }
{

}

void LButton::setPosition( float x, float y )
{
    mPosition.x = x;
    mPosition.y = y;
}

void LButton::handleEvent( SDL_Event* e )
{
    //If mouse event happened
    if ( e->type == SDL_EVENT_MOUSE_MOTION || e->type == SDL_EVENT_MOUSE_BUTTON_DOWN || e->type == SDL_EVENT_MOUSE_BUTTON_UP )
    {
        //Get mouse position
        float x = -1.f, y = -1.f;;
        SDL_GetMouseState( &x, &y );

        //check if mouse is in button
        bool inside = true;

        //mouse is left of the button
        if ( x < mPosition.x )
        {
            inside = false;
        }
        else if ( x > mPosition.x + kButtonWidth )
        {
            inside = false;
        }
        else if ( y < mPosition.y )
        {
            inside = false;
        }
        else if ( y > mPosition.y + kButtonHeight )
        {
            inside = false;
        }

        //Mouse is outside button
        if ( !inside )
        {
            mCurrentSprite = eButtonSprite::MouseOut;
        }
        //mouse is inside button
        else
        {
            switch ( e->type )
            {
                case SDL_EVENT_MOUSE_MOTION:
                mCurrentSprite = eButtonSprite::MouseOverMotion;
                break;

                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                mCurrentSprite = eButtonSprite::MouseDown;
                break;

                case SDL_EVENT_MOUSE_BUTTON_UP:
                mCurrentSprite = eButtonSprite::MouseUp;
                break;
            }
        }
    }
}

void LButton::render()
{
    //Define sprites
    SDL_FRect spriteClips[] = {
        { 0.f, 0*kButtonHeight, kButtonWidth, kButtonHeight},
        { 0.f, 1*kButtonHeight, kButtonWidth, kButtonHeight},
        { 0.f, 2*kButtonHeight, kButtonWidth, kButtonHeight},
        { 0.f, 3*kButtonHeight, kButtonWidth, kButtonHeight}
    };

    //show current button sprite
    gButtonSpriteTexture.render( mPosition.x, mPosition.y, &spriteClips[ static_cast<int>( mCurrentSprite ) ] );
}

//LTimer Implementation
LTimer::LTimer():
    mStartTicks{ 0 },
    mPausedTicks{ 0 },

    mPaused{ false },
    mStarted{ false }
{

}

void LTimer::start()
{
    //start the timer
    mStarted = true;

    //unpause the timer
    mPaused = false;

    //get the current clock time
    mStartTicks = SDL_GetTicksNS();
    mPausedTicks = 0;
}

void LTimer::stop()
{
    //Stop the timer
    mStarted = false;

    //unpause the timer
    mPaused = false;

    //clear tick variables
    mStartTicks = 0;
    mPausedTicks = 0;
}

void LTimer::pause()
{
    //if the timer is running and isnt already pasued
    if ( mStarted && !mPaused )
    {
        //pause the timer
        mPaused = true;

        //calculate the pauses ticks
        mPausedTicks = SDL_GetTicksNS() - mStartTicks;
        mStartTicks = 0;
    }
}

void LTimer::unpause()
{
    //if the timer is running and paused
    if ( mStarted && mPaused )
    {
        //unpause the timer
        mPaused = false;

        //reset the starting ticks
        mStartTicks = SDL_GetTicksNS() - mPausedTicks;

        //reset the paused ticks
        mPausedTicks = 0;
    }
}

Uint64 LTimer::getTicksNS()
{
    //the actual timer time
    Uint64 time{ 0 };

    if ( mStarted )
    {
        //if the timer is paused
        if ( mPaused )
        {
            //Return the number of ticks when the timer was paused
            time = mPausedTicks;
        }
        else
        {
            //Return the current time minus the start time
            time = SDL_GetTicksNS() - mStartTicks;
        }
    }

    return time;
}

bool LTimer::isPaused()
{
    return mPaused && mStarted;
}

bool LTimer::isStarted()
{
    return mStarted;
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
        if ( SDL_CreateWindowAndRenderer( "Eukariot", kScreenWidth, kScreenHeight, 0, &gWindow, &gRenderer ) == false )
        {
            SDL_Log("Window could not be created! SDL_Error: %s\n", SDL_GetError() );
            success = false;
        }
        else
        {
            //enable VSync
            if ( SDL_SetRenderVSync( gRenderer, 1 )  == false)
            {
                SDL_Log("Unable to set VSync! SDL Error: %s\n", SDL_GetError() );
                success = false;
            }
            //initialize font loading
            if ( TTF_Init() == false )
            {
                SDL_Log( "SDL_ttf could not initialize! SDL_ttf error: %s\n", SDL_GetError() );
                success = false;
            }
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
    if ( gPngTexture.loadFromFile( "../assets/eukariot.png", 0xFF, 0xFF, 0xFF ) == false )
    {
        SDL_Log("SDL could not load image!\n");
        success = false;
    }
    if ( gBmpTexture.loadFromFile( "../assets/sdllogo.png", 0x00, 0xFF, 0xFF ) == false )
    {
        SDL_Log("SDL could not load image!\n");
        success = false;
    }
    if ( gSpriteSheetTexture.loadFromFile( "../assets/dots.png", 0x00, 0xFF, 0xFF ) == false )
    {
        SDL_Log("SDL could not load image!\n");
        success = false;
    }
    if (gButtonSpriteTexture.loadFromFile(  "../assets/buttonmap.png", 0x00, 0xFF, 0xFF ) == false )
    {
        SDL_Log("SDL could not load image!\n");
        success = false;
    }
    std::string fontPath{ "../assets/RobotoMono-VariableFont_wght.ttf" };
    if ( gFont = TTF_OpenFont( fontPath.c_str(), 28 ); gFont == nullptr )
    {
        SDL_Log( "Could not load %s! SDL_ttf Error: %s\n", fontPath.c_str(), SDL_GetError() );
        success = false;
    }
    else
    {
        //load text
        SDL_Color textColor{ 0x00, 0x00, 0x00, 0xFF};
        if ( gTimeTextTexture.loadFromRenderedText( "Press enter to start/stop or space to pause/unpase", textColor ) == false)
        {
            SDL_Log( "Could not load text texture %s! SDL_ttf Error: %s\n", fontPath.c_str(), SDL_GetError() );
            success = false;
        }
    }

    return success;
}

void close()
{
    //clean up surface
    gPngTexture.destroy();

    //free font
    TTF_CloseFont( gFont );
    gFont = nullptr;

    //destroy window
    SDL_DestroyRenderer( gRenderer );
    gRenderer = nullptr;
    SDL_DestroyWindow( gWindow );
    gWindow = nullptr;

    //quit sdl subsystems
    TTF_Quit();
    SDL_Quit();
}