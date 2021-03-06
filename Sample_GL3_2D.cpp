#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <map>
#include <time.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <ao/ao.h>

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
map <int, Sprite> mirror;
map <string, Sprite> bucket;
map <int, Sprite> lazer;
map <int, Sprite> sboard;
int lazmir[1000][2]={0};

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
float x_change = 0; //For the camera pan
float y_change = 0; //For the camera pan
float zoom_camera = 1;
float brick_speed=-2,brick_dy=-0.5;
float degree_per_rotation=1,partition=-190,lazer_speed=20,bucket_speed=10;
double mouse_pos_x=0, mouse_pos_y=0;
double new_mouse_pos_x=0, new_mouse_pos_y=0;
double time_diff=0, current_time,old_time,laz_time,laz_old_time,m_col_time;
COLOR col[3]={black,red,green};
long long score=0,laz_no=0,mleft_click=0,mright_click=0,kleft_click=0,kright_click=0,ctrl=0,alt=0,mis_hit=6;

void create_lazer(int no);
/* Executed when a regular key is pressed/released/held-down */
void mousescroll(GLFWwindow* window, double xoffset, double yoffset)
{
    if (yoffset==-1) {
        zoom_camera /= 1.1; //make it bigger than current size
    }
    else if(yoffset==1){
        zoom_camera *= 1.1; //make it bigger than current size
    }
    if (zoom_camera<=1) {
        zoom_camera = 1;
    }
    if (zoom_camera>=4) {
        zoom_camera=4;
    }
    if(x_change-500.0f/zoom_camera<-500)
        x_change=-500+500.0f/zoom_camera;
    else if(x_change+500.0f/zoom_camera>500)
        x_change=500-500.0f/zoom_camera;
    if(y_change-350.0f/zoom_camera<-350)
        y_change=-350+350.0f/zoom_camera;
    else if(y_change+350.0f/zoom_camera>350)
        y_change=350-350.0f/zoom_camera;
    Matrices.projection = glm::ortho((float)(-500.0f/zoom_camera+x_change), (float)(500.0f/zoom_camera+x_change), (float)(-350.0f/zoom_camera+y_change), (float)(350.0f/zoom_camera+y_change), 0.1f, 500.0f);
}

//Ensure the panning does not go out of the map
void check_pan(){
    if(x_change-500.0f/zoom_camera<-500)
        x_change=-500+500.0f/zoom_camera;
    else if(x_change+500.0f/zoom_camera>500)
        x_change=500-500.0f/zoom_camera;
    if(y_change-350.0f/zoom_camera<-350)
        y_change=-350+350.0f/zoom_camera;
    else if(y_change+350.0f/zoom_camera>350)
        y_change=350-350.0f/zoom_camera;
}

/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    //for detecting multiple key press
    glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);
     // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_UP:
                mousescroll(window,0,+1);
                check_pan();
                break;
            case GLFW_KEY_DOWN:
                mousescroll(window,0,-1);
                check_pan();
                break;
            case GLFW_KEY_RIGHT:
                kright_click=0;
                x_change+=10;
                check_pan();
                break;
            case GLFW_KEY_LEFT:
                kleft_click=0;
                x_change-=10;
                check_pan();
                break;
            case GLFW_KEY_V:
                y_change+=10;
                check_pan();
                break;
            case GLFW_KEY_B:
                y_change-=10;
                check_pan();
                break;
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
            case GLFW_KEY_N:
                if(brick_speed-brick_dy>-8)
                  brick_speed+=brick_dy;
                break;
            case GLFW_KEY_M:
                if(brick_speed-brick_dy<-2)
                  brick_speed-=brick_dy;
                break;
            case GLFW_KEY_SPACE:
                if(current_time-laz_old_time>1)
                {
                  create_lazer(laz_no/2);
                  laz_no++;
                  laz_old_time=current_time;
                }
                break;
            case GLFW_KEY_LEFT_CONTROL:
                ctrl=0;
                break;
            case GLFW_KEY_RIGHT_CONTROL:
                ctrl=0;
                break;
            case GLFW_KEY_LEFT_ALT:
                alt=0;
                break;
            case GLFW_KEY_RIGHT_ALT:
                alt=0;
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
          case GLFW_KEY_A:
              cannon["front"].key_press=1;
              break;
          case GLFW_KEY_D:
              cannon["front"].key_press=2;
              break;
          case GLFW_KEY_LEFT_CONTROL:
              ctrl=1;
              break;
          case GLFW_KEY_RIGHT_CONTROL:
              ctrl=1;
              break;
          case GLFW_KEY_LEFT_ALT:
              alt=1;
              break;
          case GLFW_KEY_RIGHT_ALT:
              alt=1;
              break;
          case GLFW_KEY_RIGHT:
              kright_click=1;
              break;
          case GLFW_KEY_LEFT:
              kleft_click=1;
              break;
          case GLFW_KEY_ESCAPE:
              quit(window);
              break;
          default:
                break;
        }
    }
    if(ctrl && kleft_click)
    {
      bucket["red"].dx=-1*bucket_speed;
      bucket["red"].key_press=1;
    }
    else if(ctrl && kright_click)
    {
      bucket["red"].dx=bucket_speed;
      bucket["red"].key_press=1;
    }
    else
    {
      bucket["red"].dx=0;
      bucket["red"].key_press=0;
    }

    if(alt && kleft_click)
    {
      bucket["green"].dx=-1*bucket_speed;
      bucket["green"].key_press=1;
    }
     else if(alt && kright_click)
    {
      bucket["green"].dx=bucket_speed;
      bucket["green"].key_press=1;
    }
    else
    {
      bucket["green"].dx=0;
      bucket["green"].key_press=0;
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

string move;
/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
  Sprite b1=bucket["red"],b2=bucket["green"],c1=cannon["main"],c2=cannon["front"];
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_PRESS)
            {
              mleft_click=1;
              glfwGetCursorPos(window, &new_mouse_pos_x, &new_mouse_pos_y);
              new_mouse_pos_x=new_mouse_pos_x-500;
              new_mouse_pos_y=new_mouse_pos_y*-1+350;
              if(abs(new_mouse_pos_x-b1.x)<b1.width/2 && abs(new_mouse_pos_y-b1.y)<b1.height/2)
                move="red";
              else if(abs(new_mouse_pos_x-b2.x)<b2.width/2 && abs(new_mouse_pos_y-b2.y)<b2.height/2)
                move="green";
              else if(abs(new_mouse_pos_x-c1.x)<c1.width/2 && abs(new_mouse_pos_y-c1.y)<c1.height/2)
                move="cmain";
              else
                move="dont";
            }
            if (action == GLFW_RELEASE)
            {
              mleft_click=0;
              float cur_angle;
              glfwGetCursorPos(window, &new_mouse_pos_x, &new_mouse_pos_y);
              new_mouse_pos_x=new_mouse_pos_x-500;
              new_mouse_pos_y=new_mouse_pos_y*-1+350;
              cur_angle=atan((new_mouse_pos_y-c2.y)/(new_mouse_pos_x-c2.x))*180/M_PI;
              if(new_mouse_pos_x>(c2.x) && new_mouse_pos_y>partition)
              {
                // cout<<"hello"<<cur_angle;
                if(current_time-laz_old_time>1)
                {
                  cannon["front"].rot_angle=cur_angle;
                  create_lazer(laz_no/2);
                  laz_no++;
                  laz_old_time=current_time;
                }
              }
            }
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_PRESS) {
                mright_click=1;
            }
            if (action == GLFW_RELEASE)
            {
              mright_click=0;
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
      Matrices.projection = glm::ortho((float)(-500.0f/zoom_camera+x_change), (float)(500.0f/zoom_camera+x_change), (float)(-350.0f/zoom_camera+y_change), (float)(350.0f/zoom_camera+y_change), 0.1f, 500.0f);
  //  Matrices.projection = glm::ortho(-500.0f/zoom_camera, 500.0f/zoom_camera, -350.0f/zoom_camera, 350.0f/zoom_camera, 0.1f, 500.0f);
}


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
VAO* createTriangle ()
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
  return create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
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
  if(obj.name=="mirror1" || obj.name=="mirror2" || obj.name=="mirror3" || obj.name=="mirror4" || obj.name=="sboard")
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
void display_brick(glm::mat4 VP,GLFWwindow* window)
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
              {
                score+=1;
                // cout<<"Score: "<<score<<endl;
              }
          if(match_color(brick[k].color,green))
            if(brick[k].x>bucket["green"].x-bucket["green"].width/2 && brick[k].x<bucket["green"].x+bucket["green"].width/2)
            {
              score+=1;
              // cout<<"Score: "<<score<<endl;
            }
          if(match_color(brick[k].color,black))
          if(brick[k].x>bucket["green"].x-bucket["green"].width/2 && brick[k].x<bucket["green"].x+bucket["green"].width/2
          || (brick[k].x>bucket["red"].x-bucket["red"].width/2 && brick[k].x<bucket["red"].x+bucket["red"].width/2) )
          {
            cout<<"Final score is: "<<score<<endl;
            cout<<"Game Over\n";
            quit(window);
          }
        }
      }
    }
}
void display_buckets(glm::mat4 VP,GLFWwindow* window)
{
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
  if(mleft_click)
  {
    if(move=="red")
    {
      if(new_mouse_pos_x<(500-bucket["red"].width/2-6) && new_mouse_pos_x>(-500+bucket["red"].width/2+3) )
        bucket["red"].x=new_mouse_pos_x;
    }
    else if(move=="green")
    {
      if(new_mouse_pos_x<(500-bucket["green"].width/2-6) && new_mouse_pos_x>(-500+bucket["green"].width/2+3) )
       bucket["green"].x=new_mouse_pos_x;
    }
  }

  display(bucket["green"],VP);
  display(bucket["red"],VP);
}
/* Edit this function according to your assignment */
long long st1=0,st2=0;
void detect_collision(GLFWwindow* window)
{
  long long li,bi;
  float temp=glfwGetTime();
  float dis,dis1,dis2;
  //if(current_time-temp>5)
  //st1++;mis_hit<<
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
          dis1=lobj.height*abs(cos(lobj.rot_angle*M_PI/180)/2) + bobj.width/2;
          dis2=lobj.height*abs(sin(lobj.rot_angle*M_PI/180)/2) + bobj.height/2;
          if(dis<dis1 || dis<dis2)
          {
            if(match_color(bobj.color,black))
              {
                score+=1;
                // cout<<"Score: "<<score<<endl;
              }
            if(match_color(bobj.color,red) || match_color(bobj.color,green))
              {
                score-=1;
                mis_hit--;
                cout<<"miss hits remaining: "<<mis_hit<<endl;
                // cout<<"Score: "<<score<<endl;
                if(mis_hit==0)
                {
                  cout<<"Final score is: "<<score<<endl;
                  cout<<"Game Over\n";
                  quit(window);
                }
              }
            lazer[li].status=0;
            reset_brick(bi);
          }
        }
      }
  }
}

void check_mirror_col(int li)
{
  for (map<int,Sprite>::iterator it = mirror.begin();it!=mirror.end();it++)
    {
      int current=it->first;
      Sprite laz=lazer[li],mir=mirror[current];
      if(laz.x>mir.x-mir.width*0.5*abs(cos(mir.rot_angle*M_PI/180)) && laz.x<mir.x+mir.width*0.5*abs(cos(mir.rot_angle*M_PI/180)) &&
        laz.y>mir.y-mir.width*0.5*abs(sin(mir.rot_angle*M_PI/180)) && laz.y<mir.y+mir.width*0.5*abs(sin(mir.rot_angle*M_PI/180)) )
      {
      //  int current=1;
        // cout << "mirror" << current<< endl;
        // float dis1,dis2;
        // cout<<cos(lazer[li].rot_angle*M_PI/180.0f)<<" "<<sin(lazer[li].rot_angle*M_PI/180.0f)<<"\n";
        // // cout<<cos(mirror[current].rot_angle*M_PI/180.0f)<<" "<<sin(mirror[current].rot_angle*M_PI/180.0f)<<"\n";
        // dis1=lazer[li].x+lazer[li].width*0.5*cos(lazer[li].rot_angle*M_PI/180.0f);
        // dis2=lazer[li].y+lazer[li].width*0.5*sin(lazer[li].rot_angle*M_PI/180.0f);
        // float dis3=(dis1-mirror[current].x)/cos(mirror[current].rot_angle*M_PI/180.0f);
        // float dis4=(dis2-mirror[current].y)/sin(mirror[current].rot_angle*M_PI/18.0f);
        // //cout<<dis3<<" "<<dis4<<" \n";
        //  if (dis3 > -1*mirror[current].width/2 && dis3 < mirror[current].width/2 &&
        // dis4 > -1*mirror[current].width/2 && dis4 < mirror[current].width/2)
        // {
        //     cout << "collide mirror" << endl;
        //     lazer[li].rot_angle = 2*mirror[current].rot_angle - lazer[li].rot_angle;
        // }
          // float mul1,mul2;
          // mul1=(laz.y+laz.width/2*sin(laz.rot_angle*M_PI/180))-(tan(mir.rot_angle*M_PI/180)*(laz.x+laz.width/2*cos(laz.rot_angle*M_PI/180)))-
          //       (mir.y+mir.width/2*sin(mir.rot_angle*M_PI/180))-(tan(mir.rot_angle*M_PI/180)*(mir.x+mir.width/2)*cos(mir.rot_angle*M_PI/180));
          // mul2=(laz.y-laz.width/2*sin(laz.rot_angle*M_PI/180))-(tan(mir.rot_angle*M_PI/180)*(laz.x-laz.width/2*cos(laz.rot_angle*M_PI/180)))-
          //       (mir.y+mir.width/2*sin(mir.rot_angle*M_PI/180))-(tan(mir.rot_angle*M_PI/180)*(mir.x+mir.width/2)*cos(mir.rot_angle*M_PI/180));
          // if(mul1*mul2<=0)
          // if(lazmir[li][0]==0 || lazmir[current][1]==0)
          current_time=glfwGetTime();
          // cout << "collide mirror" << endl;
          if(current_time-m_col_time>0.1)
          {
            lazer[li].rot_angle = 2*mirror[current].rot_angle - lazer[li].rot_angle;
            lazmir[li][0]=1;
            lazmir[current][1]=1;
            m_col_time=current_time;
          }
        }
    }
}

void check_score(GLFWwindow* window)
{
  if(score==99)
  {
    cout<<"Congratulations! \n You Win\n";
  }
  if(score==-99)
  {
    cout<<"Oops! \n You Lose\n";
  }
  int o,t,nf=0;
  for(i=1;i<=15;i++)
    sboard[i].status=0;
  if(score<0)
    {
      sboard[15].status=1;
      score*=-1;
    }
  o=score%10;
  t=score/10;
  if(sboard[15].status)
    score*=-1;
  if(o==0 || o==2 || o==3 || o==5 || o==6 || o==7 || o==8 || o==9)
    sboard[1].status=1;
  if(o==0 || o==1 || o==2 || o==3 || o==4 || o==7 || o==8 || o==9)
    sboard[2].status=1;
  if(o==0 || o==1 || o==3 || o==4 || o==5 || o==6 || o==7 || o==8 || o==9)
    sboard[3].status=1;
  if(o==0 ||o==2 || o==3 || o==5 || o==6 || o==8 || o==9)
    sboard[4].status=1;
  if(o==0 || o==2 || o==6 || o==8)
    sboard[5].status=1;
  if(o==0  || o==4 || o==5 || o==6 || o==8 || o==9)
    sboard[6].status=1;
  if( o==2 || o==3 || o==4 || o==5 || o==6 || o==8 || o==9 )
    sboard[7].status=1;
  if(t==0 || t==2 || t==3 || t==5 || t==6 || t==7 || t==8 || t==9)
    sboard[8].status=1;
  if(t==0 || t==1 || t==2 || t==3 || t==4 || t==7 || t==8 || t==9)
    sboard[9].status=1;
  if(t==0 || t==1 || t==3 || t==4 || t==5 || t==6 || t==7 || t==8 || t==9)
    sboard[10].status=1;
  if(t==0 ||t==2 || t==3 || t==5 || t==6 || t==8 || t==9)
    sboard[11].status=1;
  if(t==0 || t==2 || t==6 || t==8)
    sboard[12].status=1;
  if(t==0 || t==4 || t==5 || t==6 || t==8 || t==9)
    sboard[13].status=1;
  if( t==2 || t==3 || t==4 || t==5 || t==6 || t==8 || t==9 )
    sboard[14].status=1;
}

void draw (GLFWwindow* window){
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
  if(mleft_click || mright_click)
  {
    glfwGetCursorPos(window, &new_mouse_pos_x, &new_mouse_pos_y);
    new_mouse_pos_x=new_mouse_pos_x-500;
    new_mouse_pos_y=new_mouse_pos_y*-1+350;
  }
  if(mright_click==1)
  {
    glfwGetCursorPos(window, &new_mouse_pos_x, &new_mouse_pos_y);
      x_change+=new_mouse_pos_x-mouse_pos_x;
      y_change-=new_mouse_pos_y-mouse_pos_y;
      check_pan();
  }
  Matrices.projection = glm::ortho((-500.0f/zoom_camera+x_change), (500.0f/zoom_camera+x_change), (-350.0f/zoom_camera+y_change),(350.0f/zoom_camera+y_change), 0.1f, 500.0f);
  glfwGetCursorPos(window, &mouse_pos_x, &mouse_pos_y);

  detect_collision(window);
  // cout<<current_time<<" "<<m_col_time<<endl;
    // cout<<" checking collision \n";
    for(int li=0;li<laz_no;li++)
    {
      if(lazer[li].status)
        check_mirror_col(li);
    }
  if((cannon["main"].y<(350-cannon["main"].width/2-1) && cannon["main"].dy>0) ||
     (cannon["main"].y>(partition+cannon["main"].width/2+1) && cannon["main"].dy<0))
  { cannon["main"].y+=cannon["main"].dy;
    cannon["front"].y+=cannon["main"].dy;
  }
  if(move=="cmain" && mleft_click==1)
  {
    if(new_mouse_pos_y<(350-cannon["main"].width/2-1) &&
       new_mouse_pos_y>(partition+cannon["main"].width/2+1))
        cannon["main"].y=new_mouse_pos_y;
        cannon["front"].y=cannon["main"].y;
  }

  for(int i=0;i<laz_no;i++)
  if(lazer[i].status)
  {
    lazer[i].dx=lazer_speed*cos(lazer[i].rot_angle*M_PI/180);
    lazer[i].dy=lazer_speed*sin(lazer[i].rot_angle*M_PI/180);
    lazer[i].x+=lazer[i].dx;
    lazer[i].y+=lazer[i].dy;
    if(lazer[i].x-lazer[i].width>500 || lazer[i].y-lazer[i].height>350
      ||  lazer[i].x-lazer[i].width<-550 || lazer[i].y-lazer[i].height<partition)
    {
      lazer[i].status=0;
    }
    display(lazer[i],VP);
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
  display_buckets(VP,window);
  display_brick(VP,window);
  display(mirror[1],VP);
  display(mirror[3],VP);
  display(mirror[2],VP);
  display(mirror[4],VP);
  // score=-88;
  check_score(window);
  for(int i=1;i<=15;i++)
  {
   if(sboard[i].status)
      display(sboard[i],VP);
  }
  //cout<<score<<endl;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
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

    /* Register function to handle window resi  if(no==1)

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
    glfwSetScrollCallback(window, mousescroll); // mouse scroll

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
  cannon["main"].height=40;
  cannon["main"].object = createRectangle (blue, cannon["main"].height,cannon["main"].width);
  cannon["main"].x=-500+cannon["main"].width/2;
  cannon["main"].y=0;
  cannon["front"].name="gun";
  cannon["front"].dx=0;
  cannon["front"].dy=0;
  cannon["front"].color=darkbrown;
  cannon["front"].width=40;
  cannon["front"].height=20;
  cannon["front"].rot_angle=0;
  cannon["front"].object = createRectangle (darkbrown, cannon["front"].height,cannon["front"].width);
  cannon["front"].x=-500+cannon["main"].width+cannon["front"].width/2-10;
  cannon["front"].y=0;
}

void create_lazer(int no)
{
  lazer[no].name="lazer";
  lazer[no].color=lightblue;
  lazer[no].width=100;
  lazer[no].height=5;
  lazer[no].object = createRectangle (lightblue, lazer[no].height,lazer[no].width);
  lazer[no].x=cannon["front"].x;
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
  mirror[1].name="mirror1";
  mirror[1].color=mirror_col;
  mirror[1].width=100;
  mirror[1].height=3;
  mirror[1].rot_angle=45;
  mirror[1].object = createRectangle (mirror_col, mirror[1].height,mirror[1].width);
  mirror[1].x=420;
  mirror[1].y=-130;
  mirror[1].status=0;
  mirror[1].dx=0;
  mirror[1].dy=0;

  mirror[2].name="mirror2";
  mirror[2].color=mirror_col;
  mirror[2].width=100;
  mirror[2].height=3;
  mirror[2].rot_angle=-45;
  mirror[2].object = createRectangle (mirror_col, mirror[2].height,mirror[2].width);
  mirror[2].x=420;
  mirror[2].y=200;
  mirror[2].status=0;
  mirror[2].dx=0;
  mirror[2].dy=0;

  mirror[3].name="mirror3";
  mirror[3].color=mirror_col;
  mirror[3].width=100;
  mirror[3].height=3.5;
  mirror[3].rot_angle=-60;
  mirror[3].object = createRectangle (mirror_col, mirror[3].height,mirror[3].width);
  mirror[3].x=0;
  mirror[3].y=300;
  mirror[3].status=0;
  mirror[3].dx=0;
  mirror[3].dy=0;

  mirror[4].name="mirror4";
  mirror[4].color=mirror_col;
  mirror[4].width=100;
  mirror[4].height=3.5;
  mirror[4].rot_angle=25;
  mirror[4].object = createRectangle (mirror_col, mirror[4].height,mirror[4].width);
  mirror[4].x=0;
  mirror[4].y=-10;
  mirror[4].status=0;
  mirror[4].dx=0;
  mirror[4].dy=0;
}

void create_board(int no)
{
  sboard[no].name="sboard";
  sboard[no].color=black;
  sboard[no].width=3;
  sboard[no].height=50;
  sboard[no].status=0;
  sboard[no].rot_angle=0;
  if(no==1)
  {
    sboard[no].rot_angle=90;
    sboard[no].height=35;
    sboard[no].x=478;
    sboard[no].y=345;
  }
  if(no==2)
  {
    // sboard[no].rot_angle=0;
    sboard[no].x=495;
    sboard[no].y=323;
  }
  if(no==3)
  {
    sboard[no].x=495;
    sboard[no].y=268;
  }
  if(no==4)
  {
    sboard[no].rot_angle=90;
    sboard[no].height=35;
    sboard[no].x=478;
    sboard[no].y=240;
  }
  if(no==5)
  {
    sboard[no].x=460;
    sboard[no].y=268;
  }
  if(no==6)
  {
    sboard[no].x=460;
    sboard[no].y=323;
  }
  if(no==7)
  {
    sboard[no].rot_angle=90;
    sboard[no].height=35;
    sboard[no].x=478;
    sboard[no].y=295;
  }
  if(no==8)
  {
    sboard[no].rot_angle=90;
    sboard[no].height=35;
    sboard[no].x=428;
    sboard[no].y=345;
  }
  if(no==9)
  {
    // sboard[no].rot_angle=0;
    sboard[no].x=445;
    sboard[no].y=323;
  }
  if(no==10)
  {
    sboard[no].x=445;
    sboard[no].y=268;
  }
  if(no==11)
  {
    sboard[no].rot_angle=90;
    sboard[no].height=35;
    sboard[no].x=428;
    sboard[no].y=240;
  }
  if(no==12)
  {
    sboard[no].x=410;
    sboard[no].y=268;
  }
  if(no==13)
  {
    sboard[no].x=410;
    sboard[no].y=323;
  }
  if(no==14)
  {
    sboard[no].rot_angle=90;
    sboard[no].height=35;
    sboard[no].x=428;
    sboard[no].y=295;
  }
  if(no==15)
  {
    sboard[no].rot_angle=90;
    sboard[no].height=20;
    sboard[no].x=388;
    sboard[no].y=293;
  }
  sboard[no].object = createRectangle (black, sboard[no].height,sboard[no].width);
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
  for(int i=1;i<=15;i++)
    create_board(i);

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
  glfwGetCursorPos(window, &mouse_pos_x, &mouse_pos_y);
    double last_update_time = glfwGetTime();
    old_time=last_update_time;
    laz_old_time=last_update_time-0.5;
    m_col_time=last_update_time;
    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
        draw(window);

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
