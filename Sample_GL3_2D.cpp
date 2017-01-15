#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <map>
#include <time.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO { // vertex array object
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

typedef struct COLOR {
    float r;
    float g;
    float b;
} COLOR;
int match_color(COLOR c1,COLOR c2)
{
  if(abs(c1.r-c2.r)<0.00001 &&
      abs(c1.g-c2.g)<0.00001 &&
        abs(c1.b-c2.b)<0.00001)
        return 1;
    else
    return 0;

}

COLOR mirror_col = {51/255.0, 204/255.0, 255/255.0};
COLOR grey = {168.0/255.0,168.0/255.0,168.0/255.0};
COLOR gold = {218.0/255.0,165.0/255.0,32.0/255.0};
COLOR red = {255.0/255.0,51.0/255.0,51.0/255.0};
COLOR lightgreen = {57/255.0,230/255.0,0/255.0};
COLOR lightblue ={0/255.0, 170.0/255.0, 255/255.0};
COLOR darkgreen = {51/255.0,102/255.0,0/255.0};
COLOR black = {0,0,0};
COLOR blue = {0,0,1};
COLOR green = {1.0/255.0,255.0/255.0,1.0/255.0};
COLOR darkbrown = {46/255.0,46/255.0,31/255.0};
COLOR lightbrown = {95/255.0,63/255.0,32/255.0};
COLOR lightpink = {255/255.0,122/255.0,173/255.0};
COLOR darkpink = {255/255.0,51/255.0,119/255.0};
COLOR white = {255/255.0,255/255.0,255/255.0};

typedef struct Sprite {
    string name;          // name of object
    COLOR color;          // color of object
    float x,y;            // co-odinates
    VAO* object;          // shape of object
    int key_press;           // doubt???
    int status;           // for objects to be hidden initially
    float height,width;
  //  float x_speed,y_speed;
    float dx,dy;          // amount to be moved
    float rot_angle;
    int inAir;            // boolean 0 or 1
    int fixed;            // boolean 0 or 1
    int isMoving;         // boolean 0 or 1
} Sprite;

map <string, Sprite> objects;
map <string, Sprite> cannon; //Only store cannon components here
map <int, Sprite> brick;
map <string, Sprite> mirror;
map <string, Sprite> bucket;
map <int, Sprite> lazer;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/
 void create_lazer(int no);
float degree_per_rotation=1,brick_speed=-2,partition=-190,lazer_speed=20,bucket_speed=10;
double time_diff=0, current_time,old_time,laz_time,laz_old_time;
COLOR col[3]={black,red,green};
long long score=0,laz_no=0;

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    //for detecting multiple key press
    glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);
     // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_S:
                cannon["main"].dy=0;
                cannon["front"].dy=0;
                cannon["main"].key_press=0;
                cannon["front"].key_press=0;
                break;
            case GLFW_KEY_F:
                cannon["main"].dy=0;
                cannon["front"].dy=0;
                cannon["main"].key_press=0;
                cannon["front"].key_press=0;
                break;
            case GLFW_KEY_A:
                cannon["front"].key_press=0;
                break;
            case GLFW_KEY_D:
                cannon["front"].key_press=0;
                break;
            case GLFW_KEY_LEFT:
                bucket["red"].dx=0;
                bucket["red"].key_press=0;
                break;
            case GLFW_KEY_RIGHT:
                bucket["red"].dx=0;
                bucket["red"].key_press=0;
                break;
            case GLFW_KEY_UP:
                bucket["green"].dx=0;
                bucket["green"].key_press=0;
                break;
            case GLFW_KEY_DOWN:
                bucket["green"].dx=0;
                bucket["green"].key_press=0;
                break;

            case GLFW_KEY_SPACE:
                if(current_time-laz_old_time>1)
                {
                  create_lazer(laz_no/2);
                  laz_no++;
                  laz_old_time=current_time;
                }
                break;
            case GLFW_KEY_X:
                break;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
          case GLFW_KEY_S:
              cannon["main"].dy=3;
              cannon["front"].dy=3;
              cannon["main"].key_press=1;
              break;
          case GLFW_KEY_F:
              cannon["main"].dy=-3;
              cannon["front"].dy=-3;
              cannon["main"].key_press=1;
              break;
          case GLFW_KEY_LEFT:
              bucket["red"].dx=-1*bucket_speed;
              bucket["red"].key_press=1;
              break;
          case GLFW_KEY_RIGHT:
              bucket["red"].dx=bucket_speed;
              bucket["red"].key_press=1;
              break;
          case GLFW_KEY_UP:
              bucket["green"].dx=-1*bucket_speed;
              bucket["green"].key_press=1;
              break;
          case GLFW_KEY_DOWN:
              bucket["green"].dx=bucket_speed;
              bucket["green"].key_press=1;
              break;
          case GLFW_KEY_A:
              cannon["front"].key_press=1;
              break;
          case GLFW_KEY_D:
              cannon["front"].key_press=2;
              break;
          case GLFW_KEY_SPACE:
              break;
          case GLFW_KEY_ESCAPE:
                quit(window);
                break;
          default:
                break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            quit(window);
            break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE)
  //              triangle_rot_dir *= -1;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {
    //            rectangle_rot_dir *= -1;
            }
            break;
        default:
            break;
    }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views (-x,+x,-y,+y)
    Matrices.projection = glm::ortho(-500.0f, 500.0f, -350.0f, 350.0f, 0.1f, 500.0f);
}

VAO *triangle;

VAO* createLine (COLOR color,int x1,int y1,int x2,int y2)
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
   GLfloat vertex_buffer_data [] = {
    x1, y2,0, // vertex 0
    (x1+x2)/2,(y1+y2)/2,0, // vertex 1
    x2,y2,0, // vertex 2
  };

   GLfloat color_buffer_data [] = {
    color.r,color.g,color.b, // color 0
    color.r,color.g,color.b, // color 1
    color.r,color.g,color.b, // color 2
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  return create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}


// Creates the triangle object used in this sample code
void createTriangle ()
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  GLfloat vertex_buffer_data [] = {
    0, 0,0, // vertex 0
    4.0,4.0,0, // vertex 1
    -4,-4,0, // vertex 2
  };

  GLfloat color_buffer_data [] = {
    1,0,0, // color 0
    0,1,0, // color 1
    0,0,1, // color 2
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}


// Creates the rectangle object used in this sample code
VAO* createRectangle (COLOR color1, float height, float width)
{
  // GL3 accepts only Triangles. Quads are not supported
  float w=width/2,h=height/2;
  GLfloat vertex_buffer_data [] = {
      -w,-h,0, // vertex 1
      -w,h,0, // vertex 2
      w,h,0, // vertex 3

      w,h,0, // vertex 3
      w,-h,0, // vertex 4
      -w,-h,0  // vertex 1
  };

   GLfloat color_buffer_data [] = {
    color1.r,color1.g,color1.b, // color 1
    color1.r,color1.g,color1.b, // color 2
    color1.r,color1.g,color1.b, // color 3

    color1.r,color1.g,color1.b, // color 3
    color1.r,color1.g,color1.b, // color 4
    color1.r,color1.g,color1.b,  // color 1

  };

  // create3DObject creates and returns a handle to a VAO that can be used later

    return create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;
void reset_brick(int i)
{
  brick[i].status=0;
  brick[i].y=350+brick[i].height/2;
  brick[i].dx=0;
  brick[i].dy=0;
}
void create_bricks(COLOR color1,int no,float x_co)
{
  brick[no].color=color1;
  brick[no].width=25;
  brick[no].height=50;
  brick[no].object = createRectangle (color1, brick[no].height,brick[no].width);
  brick[no].x=x_co;//-380
  brick[no].y=350+brick[no].height/2;
  brick[no].dx=0;
  brick[no].dy=0;
  brick[no].status=0;
}

void display(Sprite obj,glm::mat4 VP)
{
  glm::mat4 MVP;
  Matrices.model = glm::mat4(1.0f);
  //MVP = VP * Matrices.model; // MVP = p * V * M
  glm::mat4 ObjectTransform;
  glm::mat4 translateObject = glm::translate (glm::vec3(obj.x,obj.y, 0.0f)); // glTranslatef
  glm::mat4  rotateTriangle=glm::mat4(1.0f);
  if(obj.name=="mirror1" || obj.name=="mirror2" || obj.name=="mirror3" || obj.name=="mirror4")
  {
   rotateTriangle = glm::rotate((float)(obj.rot_angle*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  }
  if(obj.name=="gun")
  {
    rotateTriangle = glm::rotate((float)(cannon["front"].rot_angle*M_PI/180.0f), glm::vec3(0,0,1));
  }
  ObjectTransform=translateObject *rotateTriangle;
  if(obj.name=="lazer")
  {
    translateObject = glm::translate (glm::vec3(obj.x,obj.y, 0.0f)); // glTranslatef
    rotateTriangle = glm::rotate((float)(obj.rot_angle*M_PI/180.0f), glm::vec3(0,0,1));
    ObjectTransform=translateObject * rotateTriangle;
  }

  Matrices.model *= ObjectTransform;
  MVP = VP * Matrices.model; // MVP = p * V * M

  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(obj.object);
}

int flag=1;
int i=10,arr[101]={0},it=0;
void display_brick(glm::mat4 VP)
{
  int rand1;
  current_time=glfwGetTime();
  if(current_time-old_time>1)
  {
    rand1=rand()%100;
    if(brick[rand1].status==0)
      brick[rand1].status=1;
    old_time=current_time;
  }
    for(int k=0; k<100 ; k++)
    {
      if(brick[k].status==1)
      {
        display(brick[k],VP);
        if(brick[k].y>partition+brick[k].height/2)
          brick[k].y+=brick_speed;
        else
        {
          reset_brick(k);
          if(match_color(brick[k].color,red))
            if(brick[k].x>bucket["red"].x-bucket["red"].width/2 && brick[k].x<bucket["red"].x+bucket["red"].width/2)
              score+=2;
          if(match_color(brick[k].color,green))
            if(brick[k].x>bucket["green"].x-bucket["green"].width/2 && brick[k].x<bucket["green"].x+bucket["green"].width/2)
              score+=2;
          if(match_color(brick[k].color,black))
            score-=2;
        }
      }
    }
}
/* Edit this function according to your assignment */
long long st1=0,st2=0;
void detect_collision()
{
  long long li,bi;
  float temp=glfwGetTime();
  float dis,dis1,dis2;
  //if(current_time-temp>5)
  //st1++;

  for(li=st1;li<laz_no;li++)
  {
    Sprite lobj=lazer[li],bobj;
    if(lobj.status==1)
      for(bi=0;bi<100;bi++)
      {
        bobj=brick[bi];
        if(bobj.status==1)
        {
          dis=sqrt((lobj.x-bobj.x)*(lobj.x-bobj.x) +(lobj.y-bobj.y)*(lobj.y-bobj.y));
          dis1=lobj.width*abs(cos(lobj.rot_angle*M_PI/180)/2) + bobj.width/2;
          dis2=lobj.height*abs(sin(lobj.rot_angle*M_PI/180)/2) + bobj.height/2;
          //cout<<dis<<" "<<dis1<<" "<<dis2<<endl;
          if(dis<dis1 || dis<dis2)
          {
            if(match_color(bobj.color,black))
              score+=4;
            if(match_color(bobj.color,red) || match_color(bobj.color,green))
              score-=2;

            cout<<"collision\n";
            lazer[li].status=0;
            reset_brick(bi);
          }
        }
      }
  }
}


void draw (){
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


/* don't disturb anything */
  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!

  glm::mat4 MVP;	// MVP = Projection * View * Model
/* till here */
/*my code in draw function starts here */
// glUniformMatrix4fv always used before calling draw function
//  Don't change unless you are sure!!

  // Load identity to model matrix
  Matrices.model = glm::mat4(1.0f);
  MVP = VP * Matrices.model; // MVP = p * V * M
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(objects["mainline"].object);

  detect_collision();
  if((cannon["main"].y<(350-cannon["main"].width/2-1) && cannon["main"].dy>0) ||
     (cannon["main"].y>(-190+cannon["main"].width/2+1) && cannon["main"].dy<0))
  { cannon["main"].y+=cannon["main"].dy;
    cannon["front"].y+=cannon["main"].dy;
  }
  for(int i=0;i<laz_no;i++)
  if(lazer[i].status)
  {
    lazer[i].dx=lazer_speed*cos(lazer[i].rot_angle*M_PI/180);
    lazer[i].dy=lazer_speed*sin(lazer[i].rot_angle*M_PI/180);
    lazer[i].x+=lazer[i].dx;
    lazer[i].y+=lazer[i].dy;
    display(lazer[i],VP);
    if(lazer[i].x-lazer[i].width>500 || lazer[i].y-lazer[i].height>350 )
  //   ||  lazer[i].x-lazer[i].width<-500 || lazer[i].y-lazer[i].height<-350)
        {
          lazer[i].status=0;
        }
  }
  if(cannon["front"].key_press)
  {
    if(cannon["front"].key_press==1 && cannon["front"].rot_angle+degree_per_rotation<89)
      cannon["front"].rot_angle+=degree_per_rotation;
    if(cannon["front"].key_press==2 && cannon["front"].rot_angle-degree_per_rotation>-89)
      cannon["front"].rot_angle-=degree_per_rotation;

  }
  display(cannon["main"],VP);
  display(cannon["front"],VP);
  //if(bucket["red"].key_press==1 || bucket["green"].key_press==1)
  if((bucket["red"].x<(500-bucket["red"].width/2-6) && bucket["red"].dx>0) ||
  (bucket["red"].x>(-500+bucket["red"].width/2+3) && bucket["red"].dx<0))
  {
    bucket["red"].x+=bucket["red"].dx;;
  }

  if((bucket["green"].x<(500-bucket["green"].width/2-6) && bucket["green"].dx>0) ||
  (bucket["green"].x>(-500+bucket["green"].width/2+4) && bucket["green"].dx<0))
  {
    bucket["green"].x+=bucket["green"].dx;;

  }
  display_brick(VP);
  display(mirror["1"],VP);
  display(mirror["3"],VP);
  display(mirror["2"],VP);
  display(mirror["4"],VP);
  display(bucket["green"],VP);
  display(bucket["red"],VP);
  cout<<score<<endl;

}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
//        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Brick Breaker ~harshit mahajan", NULL, NULL);

    if (!window) {
        glfwTerminate();
//        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

/* my defined functions for creating objects */
void create_bucket(string color)
{
  bucket[color].height=150;
  bucket[color].width=150;
  bucket[color].dx=0;
  bucket[color].dy=0;
  if(color=="red")
  {
    bucket[color].object = createRectangle (red, bucket[color].height,bucket[color].width);
    bucket[color].name = color + "_bucket";
    bucket[color].color=red;
    bucket[color].x=-200;
    bucket[color].y=-270;
  }
  if(color=="green")
    {
      bucket[color].object = createRectangle (green,bucket[color].height ,bucket[color].width);
      bucket[color].name = color + "_bucket";
      bucket[color].color=green;
      bucket[color].x=200;
      bucket[color].y=-270;
    }
  //cout<<bucket[color].name;
}

void create_cannon()
{
  cannon["main"].name="base";
  cannon["main"].dx=0;
  cannon["main"].dy=0;
  cannon["main"].color=blue;
  cannon["main"].width=50;
  cannon["main"].height=35;
  cannon["main"].object = createRectangle (blue, cannon["main"].height,cannon["main"].width);
  cannon["main"].x=-500+cannon["main"].width/2;
  cannon["main"].y=0;
  cannon["front"].name="gun";
  cannon["front"].dx=0;
  cannon["front"].dy=0;
  cannon["front"].color=blue;
  cannon["front"].width=35;
  cannon["front"].height=20;
  cannon["front"].rot_angle=0;
  cannon["front"].object = createRectangle (blue, cannon["front"].height,cannon["front"].width);
  cannon["front"].x=-500+cannon["main"].width+cannon["front"].width/2;
  cannon["front"].y=0;
}

void create_lazer(int no)
{
  lazer[no].name="lazer";
  lazer[no].color=lightblue;
  lazer[no].width=100;
  lazer[no].height=5;
  lazer[no].object = createRectangle (lightblue, lazer[no].height,lazer[no].width);
  lazer[no].x=-500+cannon["main"].width+cannon["front"].width;
  lazer[no].y=cannon["front"].y;
  lazer[no].rot_angle=cannon["front"].rot_angle;
  lazer[no].status=1;
  lazer[no].dx=0;
  lazer[no].dy=0;
}

void brick_initializer()
{
  COLOR c1;
  int j,temp,r1,r2,brick_col[10];
  for (int i = 0; i < 5; i++)
  {
    brick_col[i+4]=100+i*60;
    if(i<4)
      brick_col[i]=-300+i*60;
  }
  for (int i = 0; i < 100; i++)
  {
    r1=rand()%9;
    r2=rand()%3;
    //cout<<r1<<r2<<endl;
      create_bricks(col[r2],i,brick_col[r1]);
  }
}

void create_mirror()
{
  mirror["1"].name="mirror1";
  mirror["1"].color=mirror_col;
  mirror["1"].width=100;
  mirror["1"].height=3;
  mirror["1"].rot_angle=45;
  mirror["1"].object = createRectangle (mirror_col, mirror["1"].height,mirror["1"].width);
  mirror["1"].x=420;
  mirror["1"].y=-130;
  mirror["1"].status=0;
  mirror["1"].dx=0;
  mirror["1"].dy=0;

  mirror["2"].name="mirror2";
  mirror["2"].color=mirror_col;
  mirror["2"].width=100;
  mirror["2"].height=3;
  mirror["2"].rot_angle=-45;
  mirror["2"].object = createRectangle (mirror_col, mirror["2"].height,mirror["2"].width);
  mirror["2"].x=420;
  mirror["2"].y=200;
  mirror["2"].status=0;
  mirror["2"].dx=0;
  mirror["2"].dy=0;

  mirror["3"].name="mirror3";
  mirror["3"].color=mirror_col;
  mirror["3"].width=100;
  mirror["3"].height=3.5;
  mirror["3"].rot_angle=-60;
  mirror["3"].object = createRectangle (mirror_col, mirror["3"].height,mirror["3"].width);
  mirror["3"].x=0;
  mirror["3"].y=300;
  mirror["3"].status=0;
  mirror["3"].dx=0;
  mirror["3"].dy=0;

  mirror["4"].name="mirror4";
  mirror["4"].color=mirror_col;
  mirror["4"].width=100;
  mirror["4"].height=3.5;
  mirror["4"].rot_angle=25;
  mirror["4"].object = createRectangle (mirror_col, mirror["4"].height,mirror["4"].width);
  mirror["4"].x=0;
  mirror["4"].y=-10;
  mirror["4"].status=0;
  mirror["4"].dx=0;
  mirror["4"].dy=0;

}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
	// Create the models
  create_bucket("red");
  create_bucket("green");
  create_cannon();
  create_mirror();
  brick_initializer();

  objects["mainline"].object=createLine(black,-500,partition,500,partition); // Generate the VAO, VBOs, vertices data & copy into the array buffer

/* No change beyond this is allowed */
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (255, 255, 255, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
    srand (time(NULL));
	int width = 1000;
	int height = 700;

    GLFWwindow* window = initGLFW(width, height);


	initGL (window, width, height);

    double last_update_time = glfwGetTime();
    old_time=last_update_time;
    laz_old_time=last_update_time-0.5;
    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
        draw();

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)

        current_time = glfwGetTime(); // Time in seconds

        if ((current_time - last_update_time) >= 0.5)
        {

          last_update_time = current_time;
        }
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
