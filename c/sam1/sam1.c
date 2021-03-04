#include <stdio.h>

int main (int argc, char *argv[]);

/*****************************************************************************/
// Main function of the program, handle commandline-arguments, HELLO WORLD
/*****************************************************************************/
int main (int argc, char *argv[])
{
   //- using printf possible too, but will almost double the size
   #if defined    (__WATCOMC__)                 // Cross-compiling environment
      #if defined      (__NT__)
         puts( "Hello OpenWatcom on Win32\n");
      #elif defined   (__OS2__)
         puts( "Hello OpenWatcom on OS/2\n");
      #elif defined   (__DOS__)
         puts( "Hello OpenWatcom on DOS\n");
      #elif defined (__LINUX__)
         puts( "Hello OpenWatcom on Linux\n");
      #else
         puts( "Hello OpenWatcom on unexpected platform ...\n");
      #endif
   #else
      puts( "Hello LLVM/Clang on macOS\n");     // macOS latest compiler suite
   #endif

   return 0;
}                                               // end 'main'
/*---------------------------------------------------------------------------*/
