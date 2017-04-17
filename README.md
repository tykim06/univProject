# univProject

## Environment

 + visual studio 2010
 + vim
   + basic vim plugin : download [.vimrc](http://www.vim-bootstrap.com/)
     + pre-pared package : sudo apt-get install git exuberant-ctags ncurses-term curl
     + troubleshooting
       + syntastic need include path
         + add .syntastic_c_config, .syntastic_cpp_config to root project directory
         + add text '-I(include path)', e.g. '-Iinclude/string/'
     + ~~autocomplete plugin : [YCM](http://valloric.github.io/YouCompleteMe/#ubuntu-linux-x64)(YouCompleteMe)~~
     + ~~troubleshooting~~
       + ~~[YouCompleteMe unavailable](http://stackoverflow.com/questions/39896698/youcompleteme-unavailable-requires-vim-7-4-143)~~
       + ~~[gnome-256color not known](https://github.com/avelino/vim-bootstrap/issues/18)~~ (**overhead**)
 + cpputest
   + troubleshooting
     + export CPPUTEST_HOME
     + execute install scripts in CPPUTEST_HOME
       
## Goal

 + Cover code(cpputest, visual studio 2010)
 + Learn vim
