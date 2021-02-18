#mini-make
install:
        ra65 -o runtime.obj runtime.m65
        mv runtime.obj ../lib/runtime.run
