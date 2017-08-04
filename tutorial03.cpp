//----------------------------------------------------------------------------
// Computer Graphics HW #1: Due March 1, 2016
// Myles Johnson-Gray (mjgray@udel.edu) w/ contributions from Christopher Rasmussen (cer@cis.udel.edu)
//
// Modification of opengl-tutorial03 
//
// Shows plan view and isometric view of 2D model using triangles.
//----------------------------------------------------------------------------

// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>

// Include math package (for PI)
#define _USE_MATH_DEFINES // for C++
#include <math.h>

//----------------------------------------------------------------------------

// these along with Model matrix make MVP transform

glm::mat4 Projection;
glm::mat4 View;

// some globals necessary to get information to shaders

GLuint MatrixID;
GLuint vertexbuffer;
GLuint colorbuffer;

//----------------------------------------------------------------------------

// Draw triangle with particular modeling transformation and color (r, g, b) (in range [0, 1])
// Refers to globals in section above (but does not change them)

void draw_triangle(glm::mat4 Model, float r, float g, float b)
{
  // Our ModelViewProjection : multiplication of our 3 matrices
  glm::mat4 MVP = Projection * View * Model;

  // make this transform available to shaders  
  glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // 1st attribute buffer : vertices
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glVertexAttribPointer(0,                  // attribute. 0 to match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);
  
  // all vertices same color

  GLfloat g_color_buffer_data[] = { 
    r, g, b,
    r, g, b,
    r, g, b,
  };
  
  glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);
  
  // 2nd attribute buffer : colors
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
  glVertexAttribPointer(1,                                // attribute. 1 to match the layout in the shader.
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
			);

  // Draw the triangle !
  glDrawArrays(GL_TRIANGLES, 0, 3); // 3 indices starting at 0 -> 1 triangle
  
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
}

//----------------------------------------------------------------------------

void draw_model(float x1, float y1, float z1,
				float x2, float y2, float z2,
				float x3, float y3, float z3, glm::mat4 idMatrix,
				float r, float g, float b)
{
	const GLfloat g_vertex_buffer_data[] = { 
    x1, y1, z1,
    x2, y2, z2,
    x3, y3, z3,
	};

	const GLushort g_element_buffer_data[] = { 0, 1, 2 };
  
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
	draw_triangle(idMatrix,     // identity transform
		  r, g, b);  //color
}

int main( void )
{
  // Initialise GLFW
  if( !glfwInit() )
    {
      fprintf( stderr, "Failed to initialize GLFW\n" );
      getchar();
      return -1;
    }
  
  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL 
  
  // Open a window and create its OpenGL context
  window = glfwCreateWindow( 1024, 768, "Tutorial 03 - Matrices", NULL, NULL);
  if( window == NULL ){
    fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
    getchar();
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  
  // Initialize GLEW
  glewExperimental = true; // Needed for core profile
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    getchar();
    glfwTerminate();
    return -1;
  }
  
  // Ensure we can capture the escape key being pressed below
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
  
  // Darker blue background
  glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
  
  GLuint VertexArrayID;
  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);
  
  // Create and compile our GLSL program from the shaders
  GLuint programID = LoadShaders( "MultiColorSimpleTransform.vertexshader", "MultiColor.fragmentshader" );
  
  // Get a handle for our "MVP" uniform
  MatrixID = glGetUniformLocation(programID, "MVP");
  
  // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
  Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
  // Or, for an ortho camera :
  //Projection = glm::ortho(-50.0f,50.0f,-50.0f,50.0f,0.0f,500.0f); // In world coordinates
  
  // Camera matrix -- same for all triangles drawn
  
  ////ISOMETIRIC VIEW #1 
  //View       = glm::lookAt(glm::vec3(20,10,40), // Camera is at (4,3,3), in World Space
		//	   glm::vec3(0,10,0), // and looks at the origin
		//	   glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
		//	   );

////ISOMETIRIC VIEW #2 
//  View       = glm::lookAt(glm::vec3(-20,10,50), // Camera is at (4,3,3), in World Space
//			   glm::vec3(0,10,0), // and looks at the origin
//			   glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
//			   );

  //PLAN VIEW
  View       = glm::lookAt(glm::vec3(-10,10,50), // Camera in World Space
			   glm::vec3(-10,10,0), // and looks at the origin
			   glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
			   );
  
  // geometry of "template" triangle
  //static const GLfloat g_vertex_buffer_data[] = { 
  //  -1.0f, -1.0f, 0.0f,
  //  1.0f, -1.0f, 0.0f,
  //  0.0f,  1.0f, 0.0f,
  //};

  //static const GLushort g_element_buffer_data[] = { 0, 1, 2 };
  //
  //glGenBuffers(1, &vertexbuffer);
  //glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  //glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
  
  // handle for color information.  we don't set it here because it can change for each triangle

  glGenBuffers(1, &colorbuffer);
  
  // Model matrix -- changed for each triangle drawn
  glm::mat4 Model = glm::mat4(1.0f);
  
  do{
    
    // Clear the screen
    glClear( GL_COLOR_BUFFER_BIT );
    
    // Use our shader
    glUseProgram(programID);
    
    // set model transform and color for each triangle and draw it offscreen

	//set initial 3 vertices (red triangle)
	glm::vec4 vec1(-30.0f, 18.0f, 0.0f, 1.0f);
	glm::vec4 vec2(-29.0f, 20.0f, 0.0f, 1.0f);
	glm::vec4 vec3(-28.0f, 18.0f, 0.0f, 1.0f);

	// create first tiled triangle (green triangle)
	glm::mat4 initialTrans = glm::translate(Model, glm::vec3(1.0f, 38.0f, 0.0f));
	float angle = M_PI;		//M_PI = pi , M_PI_2 = pi/2, M_PI_4 = pi/
	glm::mat4 rotate = glm::rotate(Model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::vec4 tvec1 = initialTrans * rotate *  vec1;
	glm::vec4 tvec2 = initialTrans * rotate *  vec2;
	glm::vec4 tvec3 = initialTrans * rotate * vec3;

	//translations to set up stacked tiling
	glm::mat4 translate = glm::translate(Model, glm::vec3(2.0f, 0.0f, 0.0f));
	glm::mat4 initialTrans2 = glm::translate(Model, glm::vec3(0.0f, -2.0f, 0.0f));
	glm::mat4 translate2 = glm::translate(Model, glm::vec3(0.0f, -4.0f, 0.0f));

	//set up 2nd row
	glm::vec4 new_vec1 = initialTrans2 * vec1;
	glm::vec4 new_vec2 = initialTrans2 * vec2;
	glm::vec4 new_vec3 = initialTrans2 * vec3;
	glm::vec4 new_tvec1 = initialTrans2 * tvec1;
	glm::vec4 new_tvec2 = initialTrans2 * tvec2;
	glm::vec4 new_tvec3 = initialTrans2 * tvec3;

	////draw first triangle
	//draw_model(float(vec1[0]), float(vec1[1]), float(vec1[2]),
	//		float(vec2[0]), float(vec2[1]), float(vec2[2]),
	//		float(vec3[0]), float(vec3[1]), float(vec3[2]), Model,
	//		1.0f, 0.0f, 0.0f); //red

	for ( int i = 0; i < 10; i++ )
	{
		//glm::mat4 translate = glm::translate(Model, glm::vec3(2.0f, 0.0f, 0.0f));

		//first row...
		vec1 = translate * vec1;
		vec2 = translate * vec2;
		vec3 = translate * vec3;
		tvec1 = translate * tvec1;
		tvec2 = translate * tvec2;
		tvec3 = translate * tvec3;

		//create temp vectors
		glm::vec4 temp_vec1 = vec1;
		glm::vec4 temp_vec2 = vec2;
		glm::vec4 temp_vec3 = vec3;
		glm::vec4 temp_tvec1 = tvec1;
		glm::vec4 temp_tvec2 = tvec2;
		glm::vec4 temp_tvec3 = tvec3;

		//draw red and green triangles
		draw_model(float(vec1[0]), float(vec1[1]), float(vec1[2]),
				float(vec2[0]), float(vec2[1]), float(vec2[2]),
				float(vec3[0]), float(vec3[1]), float(vec3[2]), Model,
				1.0f, 0.0f, 0.0f); //red

		draw_model(float(tvec1[0]), float(tvec1[1]), float(tvec1[2]),
				float(tvec2[0]), float(tvec2[1]), float(tvec2[2]),
				float(tvec3[0]), float(tvec3[1]), float(tvec3[2]), Model,
				0.0f, 1.0f, 0.0f); //green

		//second row...
		new_vec1 = translate * new_vec1;
		new_vec2 = translate * new_vec2;
		new_vec3 = translate * new_vec3;
		new_tvec1 = translate * new_tvec1;
		new_tvec2 = translate * new_tvec2;
		new_tvec3 = translate * new_tvec3;

		//create temp vectors
		glm::vec4 temp_new_vec1 = new_vec1;
		glm::vec4 temp_new_vec2 = new_vec2;
		glm::vec4 temp_new_vec3 = new_vec3;
		glm::vec4 temp_new_tvec1 = new_tvec1;
		glm::vec4 temp_new_tvec2 = new_tvec2;
		glm::vec4 temp_new_tvec3 = new_tvec3;

		//draw purple and light blue
		draw_model(float(new_vec1[0]), float(new_vec1[1]), float(new_vec1[2]),
				float(new_vec2[0]), float(new_vec2[1]), float(new_vec2[2]),
				float(new_vec3[0]), float(new_vec3[1]), float(new_vec3[2]), Model,
				1.0f, 0.0f, 1.0f); //purple

		draw_model(float(new_tvec1[0]), float(new_tvec1[1]), float(new_tvec1[2]),
				float(new_tvec2[0]), float(new_tvec2[1]), float(new_tvec2[2]),
				float(new_tvec3[0]), float(new_tvec3[1]), float(new_tvec3[2]), Model,
				0.0f, 1.0f, 1.0f); //light blue

		for  ( int j = 0; j < 5; j++ )
		{
			//repeat pattern vertically

				////set scaling matrix
				glm::mat4 scale = glm::scale(Model, glm::vec3(0.9f, 1.0f, 1.0f));

				//scale and translate triangles to create additional rows
				temp_vec1 = translate2 * scale * temp_vec1;
				temp_vec2 = translate2 * scale * temp_vec2;
				temp_vec3 = translate2 * scale * temp_vec3;

				temp_tvec1 = translate2 * scale * temp_tvec1;
				temp_tvec2 = translate2 * scale * temp_tvec2;
				temp_tvec3 = translate2 * scale * temp_tvec3;

				temp_new_vec1 = translate2 * scale * temp_new_vec1;
				temp_new_vec2 = translate2 * scale * temp_new_vec2;
				temp_new_vec3 = translate2 * scale * temp_new_vec3;

				temp_new_tvec1 = translate2 * scale * temp_new_tvec1;
				temp_new_tvec2 = translate2 * scale * temp_new_tvec2;
				temp_new_tvec3 = translate2 * scale * temp_new_tvec3;

				//draw triangles
				draw_model(float(temp_vec1[0]), float(temp_vec1[1]), float(temp_vec1[2]),
					float(temp_vec2[0]), float(temp_vec2[1]), float(temp_vec2[2]),
					float(temp_vec3[0]), float(temp_vec3[1]), float(temp_vec3[2]), Model,
					1.0f, 0.0f, 0.0f); //red

				draw_model(float(temp_tvec1[0]), float(temp_tvec1[1]), float(temp_tvec1[2]),
					float(temp_tvec2[0]), float(temp_tvec2[1]), float(temp_tvec2[2]),
					float(temp_tvec3[0]), float(temp_tvec3[1]), float(temp_tvec3[2]), Model,
					0.0f, 1.0f, 0.0f); //green

				draw_model(float(temp_new_vec1[0]), float(temp_new_vec1[1]), float(temp_new_vec1[2]),
					float(temp_new_vec2[0]), float(temp_new_vec2[1]), float(temp_new_vec2[2]),
					float(temp_new_vec3[0]), float(temp_new_vec3[1]), float(temp_new_vec3[2]), Model,
					1.0f, 0.0f, 1.0f); //purple

				draw_model(float(temp_new_tvec1[0]), float(temp_new_tvec1[1]), float(temp_new_tvec1[2]),
					float(temp_new_tvec2[0]), float(temp_new_tvec2[1]), float(temp_new_tvec2[2]),
					float(temp_new_tvec3[0]), float(temp_new_tvec3[1]), float(temp_new_tvec3[2]), Model,
					0.0f, 1.0f, 1.0f); //light blue	
		}
	}
	// more/other calls to draw_triangle() ...

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();

  } // Check if the ESC key was pressed or the window was closed
  while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
	 glfwWindowShouldClose(window) == 0 );
  
  // Cleanup VBO and shader
  glDeleteBuffers(1, &vertexbuffer);
  glDeleteProgram(programID);
  glDeleteVertexArrays(1, &VertexArrayID);
  
  // Close OpenGL window and terminate GLFW
  glfwTerminate();
  
  return 0;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------