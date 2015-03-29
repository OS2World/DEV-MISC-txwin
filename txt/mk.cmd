/* make, for 32 bit OS/2 environment      STD, TRACE and DEBUG version       */

   arg debug options

   mkstd = 'yes'
   if (substr(debug,1,1) <> '/') & (substr(debug,1,1) <> '-') then
   do
      if length(debug) = 1 then
      do
         mkstd = ''
      end
      mkopt = options
   end; else
   do
      mkopt = debug options
      debug = ''
   end

   mkdef = '@nmake /nologo' mkopt '/f makefile.32b /x mk.err'

   old = Directory()

   if debug <> '' then
   do
      select
      when substr(debug,1,1) == 'I' then        /* IPMD debug only, no trace */
         do
            Address cmd mkdef 'CODEVIEW=y'
            if rc=0 then
            do
               Address cmd '@copy cvo32bin\txt.exe txtipmd.exe'
            end
         end
      when substr(debug,1,1) == 'T' then        /* TRACE only, no IPMD debug */
         do
            Address cmd mkdef 'DEBUGGIN=y'
            if rc=0 then
            do
               Address cmd '@copy dbo32bin\txt.exe txttros2.exe'
               Address cmd 'eautil txttros2.exe txttros2.ea /S'
               Address cmd 'del txttros2.ea'
               Address cmd 'lxlite txttros2.exe'
            end
         end
      otherwise                                 /* TRACE and IPMD debug      */
         Address cmd mkdef 'DEBFLG=-DDUMP CODEVIEW=y'
         if rc=0 then
         do
            Address cmd '@copy cvo32bin\txt.exe tx.exe'
         end
         nop
      end
   end

   if mkstd <> ''  then
   do
      Address cmd mkdef
      if rc=0 then
      do
         Address cmd '@copy o32bin\txt.exe txt.exe'
         Address cmd 'eautil txt.exe txt.ea /S'
         Address cmd 'del txt.ea'
         Address cmd 'lxlite txt.exe'
         Say 'TXT.EXE has been compressed! (LXLITE)'
      end
   end

   now = Directory(old)
