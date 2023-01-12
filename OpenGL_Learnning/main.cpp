#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#pragma warning(disable: 4996)

// Transform
int x_rotation_times = 0.0f;
bool bRotateX = false;
bool bRotateY = false;
bool bRotateZ = true;

// Colour
float gradient_visibility = 1.0f;
float texture_visibility = 1.0f;
float both_visibility = 1.0f;
float pClearColor[] = { 0.0f, 0.0f, 0.0f };

bool bEnableTexture = true;
bool bEnableGradient = false;
bool bAnimateVisibility = false;

// Primitive
bool bEnablePrimitives = false;
int current_prim = 0;
int point_size = 1;

struct Vector3f
{
	float x, y, z;
};

struct Vector2f
{
	float x, y;
};

struct SingleVertex
{
	Vector3f position;
	Vector3f colour;
	Vector2f texcoord;
};

void Callback_ResizeFramebuffer(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	return;
}

void LoadShader(const char* path, char** buffer)
{
	std::ifstream file;
	file.open(path);
	if (file.is_open())
	{
		std::stringstream ss;
		ss << file.rdbuf();

		*buffer = new char[ss.str().length()];
		strcpy(*buffer, ss.str().c_str());
	}
	else
		std::cout << "Cannot open shader source!" << std::endl;
	return;
}

unsigned int CreateShader(const char* path, std::uint32_t type)
{
	unsigned int id = glCreateShader(type);
	char* shader_src = nullptr;
	LoadShader(path, &shader_src);
	glShaderSource(id, 1, &shader_src, nullptr);
	glCompileShader(id);

	int status;
	glGetShaderiv(id, GL_COMPILE_STATUS, &status);

	if (!status)
	{
		int len;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
		if (len)
		{
			char* log_buffer = new char[len + 1];
			glGetShaderInfoLog(id, len, &len, log_buffer);
			std::cout << ((type == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT");
			std::cout << " SHADER COMPILE STATUS: " << log_buffer << std::endl;
			delete[] log_buffer;

			glDeleteShader(id);
			return 0u;
		}
	}

	return id;
}

unsigned int InitProgram(std::uint32_t vs, std::uint32_t fs)
{
	unsigned int id = glCreateProgram();

	glAttachShader(id, vs);
	glAttachShader(id, fs);
	
	glValidateProgram(id);
	glLinkProgram(id);

	return id;
}

int main(int argc, char** argv)
{
	if (!glfwInit())
	{
		std::cout << "Cannot initialize GLFW." << std::endl;
		return -1;
	}

	glfwInitHint(GLFW_VERSION_MINOR, 3);
	glfwInitHint(GLFW_VERSION_MAJOR, 3);
	glfwInitHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(640, 480, "OpenGL Learning", nullptr, nullptr);
	if (!window)
	{
		std::cout << "Cannot create window!" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwSetFramebufferSizeCallback(window, &::Callback_ResizeFramebuffer);

	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK)
	{
		std::cout << "Cannot initialize GLEW." << std::endl;
		glfwTerminate();
		return -1;
	}

	SingleVertex SingleVertex[] = {
		{ { -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f },	{ 0.0f, 0.0f } },
		{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f },	{ 1.0f, 0.0f } },
		{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f },	{ 0.0f, 1.0f } },
		{ {  0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f, 0.0f },	{ 1.0f, 1.0f } },

		{ { -0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f },	{ 0.0f, 0.0f } },
		{ {  0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f },	{ 1.0f, 0.0f } },
		{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f },	{ 0.0f, 1.0f } },
		{ {  0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f, 0.0f },	{ 1.0f, 1.0f } }
	};

	unsigned int indices[] = {
		0, 1, 2,
		1, 3, 2,

		4, 5, 6,
		6, 7, 5
	};

	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(SingleVertex), SingleVertex, GL_STATIC_DRAW);

	unsigned int ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	unsigned int vs = CreateShader("vertex_shader.vert", GL_VERTEX_SHADER);
	unsigned int fs = CreateShader("fragment_shader.frag", GL_FRAGMENT_SHADER);

	if (!(vs && fs))
	{
		std::cout << "Shader initialization failed.\nTerminating program." << std::endl;
		glfwTerminate();
		return -1;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	unsigned int program_id = InitProgram(vs, fs);
	glDeleteShader(vs);
	glDeleteShader(fs);

	glUseProgram(program_id);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)0u);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 3));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 6));
	glEnableVertexAttribArray(2);


	unsigned int tex_id;
	glGenTextures(1, &tex_id);
	glBindTexture(GL_TEXTURE_2D, tex_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	int w, h, c;
	unsigned char* data = stbi_load("wall.jpg", &w, &h, &c, 0);
	if (data)
	{
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RGB,
			w, h, 0,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			data
		);

		glGenerateMipmap(GL_TEXTURE_2D);
	}

	stbi_image_free(data);

	glUseProgram(0u);
	glBindBuffer(GL_ARRAY_BUFFER, 0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);
	glBindTexture(GL_TEXTURE_2D, 0u);

	while (!glfwWindowShouldClose(window))
	{
		glClearColor(pClearColor[0], pClearColor[1], pClearColor[2], 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		
		ImGui::Begin("Control Panel");
		bool bShowTransform = ImGui::CollapsingHeader("Transform");
		if (bShowTransform)
		{
			ImGui::Text("Rotation Speed: ");
			ImGui::SameLine();
			if (x_rotation_times == 0)
				ImGui::SliderInt("##rotation_x_times", &x_rotation_times, 0, 10, "Rotation Speed: None");
			else if (x_rotation_times == 10)
				ImGui::SliderInt("##rotation_x_times", &x_rotation_times, 0, 10, "Rotation Speed: TOO FAST");
			else
				ImGui::SliderInt("##rotation_x_times", &x_rotation_times, 0, 10, "Rotation Speed: x%d");
			
			ImGui::Separator();

			if (ImGui::Button("Check All"))
			{
				bRotateX = true;
				bRotateY = true;
				bRotateZ = true;
			}

			ImGui::Checkbox("Rotate X", &bRotateX);
			ImGui::Checkbox("Rotate Y", &bRotateY);
			ImGui::Checkbox("Rotate Z", &bRotateZ);

			if (!bRotateX && !bRotateY && !bRotateZ)
				bRotateZ = true;
			ImGui::Separator();
		}

		bool bShowColour = ImGui::CollapsingHeader("Colour");
		if (bShowColour)
		{
			ImGui::ColorPicker3("##color_pick", pClearColor);
			ImGui::Separator();
			ImGui::Checkbox("Enable Texture", &bEnableTexture);
			ImGui::Checkbox("Enable Gradient", &bEnableGradient);

			ImGui::Separator();

			if (bEnableTexture && !bEnableGradient)
			{
				ImGui::Text("Texture Visibility: ");
				ImGui::SameLine();
				ImGui::SliderFloat("##tex_vis", &texture_visibility, .0f, 1.0f, "%f");
				bEnableGradient = false;
			}
			else if (bEnableGradient && !bEnableTexture)
			{
				ImGui::Text("Gradient Visibility: ");
				ImGui::SameLine();
				ImGui::SliderFloat("##tex_vis", &gradient_visibility, .0f, 1.0f, "%f");
				bEnableTexture = false;
			}
			else
			{
				ImGui::Text("Visibility: ");
				ImGui::SameLine();
				ImGui::SliderFloat("##both_vis", &both_visibility, .0f, 1.0f, "%f");
				ImGui::Checkbox("Animate Visibility", &bAnimateVisibility);
			}
		}

		bool bShowPrimitives = ImGui::CollapsingHeader("Primitives");
		if (bShowPrimitives)
		{
			ImGui::Checkbox("Enable Primitives", &bEnablePrimitives);
			ImGui::Separator();

			const char* items[] = {
				"Triangle",
				"Line",
				"Point"
			};

			ImGui::ListBox("##prims_list", &current_prim, items, 3);
			if (current_prim == 2)
			{
				ImGui::Separator();
				ImGui::SliderInt("##ptsize", &point_size, 0, 20, "Point Size: %d");
			}
		}

		ImGui::End();

		static bool bAnimateVisibModeNeg = false;
		if (bAnimateVisibility)
		{
			if (!bAnimateVisibModeNeg)
			{
				if (both_visibility >= 1.0f)
					bAnimateVisibModeNeg = true;
				else
					both_visibility += 0.01f;
			}
			else
			{
				both_visibility -= 0.01f;
				if (both_visibility <= 0.0f)
					bAnimateVisibModeNeg = false;
			}
		}

		glEnable(GL_DEPTH_TEST);
		// Render here...
		glUseProgram(program_id);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBindTexture(GL_TEXTURE_2D, tex_id);

		glm::mat4 trans = glm::mat4(1.0f);
		trans = glm::rotate(trans, (float)glfwGetTime() * (float)x_rotation_times, glm::vec3((float)bRotateX, (float)bRotateY, (float)bRotateZ));
		int tloc = glGetUniformLocation(program_id, "transform");
		int cloc = glGetUniformLocation(program_id, "col_mul");
		int gloc = glGetUniformLocation(program_id, "grad_mul");

		int disable_texture_loc = glGetUniformLocation(program_id, "disableTexture");
		int enable_gradient_loc = glGetUniformLocation(program_id, "enableGradient");
		int enable_both_loc = glGetUniformLocation(program_id, "enableBoth");
		int visibility_both_loc = glGetUniformLocation(program_id, "visibility_both");

		float rgb_mul[4] = { gradient_visibility, gradient_visibility, gradient_visibility, 0.0f };
		float rgb_mul_tex[4] = { texture_visibility, texture_visibility, texture_visibility, 0.0f };

		glUniform4fv(cloc, 1, rgb_mul_tex);
		glUniform4fv(gloc, 1, rgb_mul);
		glUniform1f(visibility_both_loc, both_visibility);
		glUniformMatrix4fv(tloc, 1, GL_FALSE, glm::value_ptr(trans));

		if (bEnableTexture)
			glUniform1i(disable_texture_loc, 0);
		else
			glUniform1i(disable_texture_loc, 1);

		if (bEnableGradient)
			glUniform1i(enable_gradient_loc, 1);
		else
			glUniform1i(enable_gradient_loc, 0);

		if(bEnableGradient && bEnableTexture)
			glUniform1i(enable_both_loc, 1);
		else
			glUniform1i(enable_both_loc, 0);

		unsigned int primsa[] = { GL_FILL, GL_LINE, GL_POINT };
		if (bEnablePrimitives)
		{
			glEnable(GL_POLYGON_MODE);
			glEnable(GL_POINT_SIZE);
			glPolygonMode(GL_FRONT_AND_BACK, primsa[current_prim]);
			glPointSize((float)point_size);
		}

		glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_INT, nullptr);

		glDisable(GL_POLYGON_MODE);
		glDisable(GL_POINT_SIZE);
		glDisable(GL_DEPTH_TEST);

		ImGui::Render();
		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}