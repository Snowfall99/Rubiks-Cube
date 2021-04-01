# Rubiks Cube 
This is the self-project for IS305, 2021 spring.  
Using OpenGL to render a Rubiks Cube.

## Version 1
Using GL/gl.h, which is easy to use in certain aspects, however, this lib has too many restrictions which limit further optimizations and further development. As a consequence, 
I will turn to glad in the next version, despite of the fact that I only have little time to develop this project.

## Version 2
Using glad to render 27 blocks in order to construct the cube. Render a skybox for background.  

**Todo List**  
- [ ] Edge display
- [x] skybox render 
- [ ] face state display 
- [x] Human-computer interaction page
- [x] generate a random state cube
- [ ] State reset
- [x] read state from file and generate cube based on it
- [ ] auto-solve method
- [ ] Optimize URF move
- [ ] Using dialog box to show error messages
