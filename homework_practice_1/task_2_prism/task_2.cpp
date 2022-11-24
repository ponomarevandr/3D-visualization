#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
//using namespace glm;

#include <common/shader.hpp>

const float PI = 3.1415926536;
const float camera_speed = PI / 200;
const float figure_speed = PI / 110;
float camera_angle = 0;
float figure_angle = 0;

//Массивы координат и цветов точек отрисовываемых объектов
const float PRISM_X_1 = 0.0f;
const float PRISM_X_2 = std::cos(PI / 6);
const float PRISM_X_3 = -std::cos(PI / 6);
const float PRISM_Z_1 = 1.0f;
const float PRISM_Z_2 = -std::sin(PI / 6);
const float PRISM_Z_3 = -std::sin(PI / 6);
const GLfloat triangles_vertices[] = {
	PRISM_X_1, -1.0f, PRISM_Z_1,
	PRISM_X_2, -1.0f, PRISM_Z_2,
	PRISM_X_3, -1.0f, PRISM_Z_3,
	PRISM_X_1, 1.0f, PRISM_Z_1,
	PRISM_X_2, 1.0f, PRISM_Z_2,
	PRISM_X_3, 1.0f, PRISM_Z_3,

	PRISM_X_1, -1.0f, PRISM_Z_1,
	PRISM_X_2, -1.0f, PRISM_Z_2,
	PRISM_X_2, 1.0f, PRISM_Z_2,
	PRISM_X_1, -1.0f, PRISM_Z_1,
	PRISM_X_1, 1.0f, PRISM_Z_1,
	PRISM_X_2, 1.0f, PRISM_Z_2,

	PRISM_X_2, -1.0f, PRISM_Z_2,
	PRISM_X_3, -1.0f, PRISM_Z_3,
	PRISM_X_3, 1.0f, PRISM_Z_3,
	PRISM_X_2, -1.0f, PRISM_Z_2,
	PRISM_X_2, 1.0f, PRISM_Z_2,
	PRISM_X_3, 1.0f, PRISM_Z_3,

	PRISM_X_3, -1.0f, PRISM_Z_3,
	PRISM_X_1, -1.0f, PRISM_Z_1,
	PRISM_X_1, 1.0f, PRISM_Z_1,
	PRISM_X_3, -1.0f, PRISM_Z_3,
	PRISM_X_3, 1.0f, PRISM_Z_3,
	PRISM_X_1, 1.0f, PRISM_Z_1,
};
const GLfloat triangles_colors[] = {
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f,

	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,

	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f,

	0.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 0.0f,
};

int programInitialize(GLFWwindow*& window, GLuint& vertex_buffer, GLuint& color_buffer,
		GLuint& vertex_array, GLuint& program_id) {
	// Инициализируем GLFW
	if (!glfwInit()) {
	    fprintf(stderr, "Ошибка при инициализации GLFW\n");
	    return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4); // 4x Сглаживание
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Мы хотим использовать OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Мы не хотим старый OpenGL

	// Открываем окно и создаем в нем контекст OpenGL
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	window = glfwCreateWindow(1000, 700, "Задача 2", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Невозможно открыть окно GLFW\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Инициализируем GLEW
	glewExperimental = true; // Флаг необходим в Core-режиме OpenGL
	if (glewInit() != GLEW_OK) {
	    fprintf(stderr, "Невозможно инициализировать GLEW\n");
	    getchar();
	    glfwTerminate();
	    return -1;
	}

	// Включим режим отслеживание нажатия клавиш для проверки ниже
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Включение механизма Z-буффера
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// Создание Vertex Array Object'а
	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);

	// Загрузка и компиляция шейдеров
	program_id = LoadShaders(
		"TransformVertexShader.vertexshader",
		"ColorFragmentShader.fragmentshader"
	);

	// Создание буфферов вершин и цветов
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangles_vertices), triangles_vertices,
		GL_STATIC_DRAW);
	glGenBuffers(1, &color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangles_colors), triangles_colors,
		GL_STATIC_DRAW);

	// Установим цвет фона
	glClearColor(0.0f, 0.0f, 0.2f, 0.0f);

	return 0;
}

void programFinalize(GLuint& vertex_buffer, GLuint& color_buffer, GLuint& vertex_array,
		GLuint& program_id) {
	// Очистка VBO
	glDeleteBuffers(1, &vertex_buffer);
	glDeleteBuffers(1, &color_buffer);
	glDeleteVertexArrays(1, &vertex_array);
	glDeleteProgram(program_id);

	// Закрытие окна OpenGL и уничтожение GLFW
	glfwTerminate();
}


int main() {
	GLFWwindow* window;
	GLuint vertex_buffer;
	GLuint color_buffer;
	GLuint vertex_array;
	GLuint program_id;
	if (programInitialize(window, vertex_buffer, color_buffer, vertex_array, program_id) != 0)
		return -1;

	do {
		// Формирование матрицы преобразования пространства
		GLuint matrix_id = glGetUniformLocation(program_id, "MVP");
		glm::mat4 matrix_projection =
			glm::perspective(glm::radians(60.0f), 10.0f / 7.0f, 0.1f, 100.0f);
		camera_angle += camera_speed;
		figure_angle += figure_speed;
		glm::mat4 matrix_view  = glm::lookAt(
			glm::vec3(0, 3 * std::sin(camera_angle), 3 * std::cos(camera_angle)),
			glm::vec3(0, 0, 0),
			glm::vec3(0, 3 * std::cos(camera_angle), -3 * std::sin(camera_angle))
		);
		glm::mat4 matrix_model = glm::rotate(glm::mat4(1.0f), figure_angle, glm::vec3(0, 1, 0));
		
		glm::mat4 MVP = matrix_projection * matrix_view * matrix_model;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Устанонавливаем шейдер текущим
		glUseProgram(program_id);

		// Установка матрицы преобразования пространства
		glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &MVP[0][0]);

	    // Подключение буфферов вершин и цветов
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glVertexAttribPointer(
		   0,
		   3,					// Размер
		   GL_FLOAT,			// Тип
		   GL_FALSE,			// Указывает, что значения не нормализованы
		   0,					// Шаг
		   (void*)0				// Смещение массива в буфере
		);
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
		glVertexAttribPointer(
			1,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);

		// Рисование вершин
		glDrawArrays(GL_TRIANGLES, 0, 3 * 8);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

	    // Смена буфферов
	    glfwSwapBuffers(window);
	    glfwPollEvents();

	    // Проверяем нажатие клавиши Escape или закрытие окна
	} while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

	programFinalize(vertex_buffer, color_buffer, vertex_array, program_id);

	return 0;
}