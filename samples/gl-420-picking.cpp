//**********************************
// OpenGL Samples Pack 
// ogl-samples.g-truc.net
//**********************************
// OpenGL Picking
// 27/06/2012 - 15/08/2012
//**********************************
// Christophe Riccio
// ogl-samples@g-truc.net
//**********************************
// G-Truc Creation
// www.g-truc.net
//**********************************

#include <glf/glf.hpp>

namespace
{
	std::string const SAMPLE_NAME("OpenGL Picking");
	std::string const VERT_SHADER_SOURCE(glf::DATA_DIRECTORY + "gl-420/picking.vert");
	std::string const GEOM_SHADER_SOURCE(glf::DATA_DIRECTORY + "gl-420/picking.geom");
	std::string const FRAG_SHADER_SOURCE(glf::DATA_DIRECTORY + "gl-420/picking.frag");
	std::string const TEXTURE_DIFFUSE(glf::DATA_DIRECTORY + "kueken1-bgr8.dds");
	int const SAMPLE_SIZE_WIDTH(640);
	int const SAMPLE_SIZE_HEIGHT(480);
	int const SAMPLE_MAJOR_VERSION(4);
	int const SAMPLE_MINOR_VERSION(2);

	glf::window Window(glm::ivec2(SAMPLE_SIZE_WIDTH, SAMPLE_SIZE_HEIGHT));

	GLsizei const VertexCount(4);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glf::vertex_v2fv2f);
	glf::vertex_v2fv2f const VertexData[VertexCount] =
	{
		glf::vertex_v2fv2f(glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f,-1.0f), glm::vec2(1.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 0.0f))
	};

	GLsizei const ElementCount(6);
	GLsizeiptr const ElementSize = ElementCount * sizeof(GLushort);
	GLushort const ElementData[ElementCount] =
	{
		0, 1, 2, 
		2, 3, 0
	};

	namespace buffer
	{
		enum type
		{
			VERTEX,
			ELEMENT,
			TRANSFORM,
			PICKING,
			MAX
		};
	}//namespace buffer

	namespace texture
	{
		enum type
		{
			DIFFUSE,
			PICKING,
			MAX
		};
	}//namespace texture

	GLuint PipelineName(0);
	GLuint ProgramName(0);
	GLuint VertexArrayName(0);
	GLuint BufferName[buffer::MAX] = {0, 0, 0};
	GLuint TextureName[texture::MAX] = {0, 0};
}//namespace

bool initProgram()
{
	bool Validated(true);
	
	glGenProgramPipelines(1, &PipelineName);

	if(Validated)
	{
		glf::compiler Compiler;
		GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, VERT_SHADER_SOURCE, 
			"--version 420 --profile core");
		GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, FRAG_SHADER_SOURCE,
			"--version 420 --profile core");
		Validated = Validated && Compiler.check();

		ProgramName = glCreateProgram();
		glProgramParameteri(ProgramName, GL_PROGRAM_SEPARABLE, GL_TRUE);
		glAttachShader(ProgramName, VertShaderName);
		glAttachShader(ProgramName, FragShaderName);
		glLinkProgram(ProgramName);
		Validated = Validated && glf::checkProgram(ProgramName);
	}

	if(Validated)
		glUseProgramStages(PipelineName, GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, ProgramName);

	return Validated;
}

bool initBuffer()
{
	bool Validated(true);

	glGenBuffers(buffer::MAX, BufferName);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
    glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLint UniformBufferOffset(0);

	glGetIntegerv(
		GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT,
		&UniformBufferOffset);

	GLint UniformBlockSize = glm::max(GLint(sizeof(glm::mat4)), UniformBufferOffset);

	glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
	glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBuffer(GL_TEXTURE_BUFFER, BufferName[buffer::PICKING]);
	glBufferData(GL_TEXTURE_BUFFER, sizeof(float), NULL, GL_DYNAMIC_READ);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);

	return Validated;
}

bool initTexture()
{
	bool Validated(true);

	gli::texture2D Texture = gli::load(TEXTURE_DIFFUSE);

	glGenTextures(texture::MAX, TextureName);

	// Diffuse
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureName[texture::DIFFUSE]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(Texture.levels() - 1));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexStorage2D(GL_TEXTURE_2D, GLint(Texture.levels()), GL_RGBA8, GLsizei(Texture[0].dimensions().x), GLsizei(Texture[0].dimensions().y));

		for(gli::texture2D::level_type Level = 0; Level < Texture.levels(); ++Level)
		{
			glTexSubImage2D(
				GL_TEXTURE_2D, 
				GLint(Level), 
				0, 0, 
				GLsizei(Texture[Level].dimensions().x), 
				GLsizei(Texture[Level].dimensions().y), 
				GL_BGR, GL_UNSIGNED_BYTE, 
				Texture[Level].data());
		}
	
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	}

	// Piking
	{
		glBindTexture(GL_TEXTURE_BUFFER, TextureName[texture::PICKING]);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, BufferName[buffer::PICKING]);
		glBindTexture(GL_TEXTURE_BUFFER, 0);
	}

	return Validated;
}

bool initVertexArray()
{
	glGenVertexArrays(1, &VertexArrayName);
	glBindVertexArray(VertexArrayName);
		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
		glVertexAttribPointer(glf::semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), GLF_BUFFER_OFFSET(0));
		glVertexAttribPointer(glf::semantic::attr::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), GLF_BUFFER_OFFSET(sizeof(glm::vec2)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnableVertexAttribArray(glf::semantic::attr::POSITION);
		glEnableVertexAttribArray(glf::semantic::attr::TEXCOORD);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
	glBindVertexArray(0);

	return true;
}

bool initTest()
{
	glEnable(GL_DEPTH_TEST);

	return true;
}

bool initDebugOutput()
{
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
	glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);
	glDebugMessageCallbackARB(&glf::debugOutput, NULL);

	return true;
}

bool begin()
{
	bool Validated(true);
	Validated = Validated && glf::checkGLVersion(SAMPLE_MAJOR_VERSION, SAMPLE_MINOR_VERSION);

	if(Validated && glf::checkExtension("GL_ARB_debug_output"))
		Validated = initDebugOutput();
	if(Validated)
		Validated = initTest();
	if(Validated)
		Validated = initBuffer();
	if(Validated)
		Validated = initTexture();
	if(Validated)
		Validated = initProgram();
	if(Validated)
		Validated = initVertexArray();

	return Validated;
}

bool end()
{
	bool Validated(true);

	glDeleteProgramPipelines(1, &PipelineName);
	glDeleteProgram(ProgramName);
	glDeleteBuffers(buffer::MAX, BufferName);
	glDeleteTextures(texture::MAX, TextureName);
	glDeleteVertexArrays(1, &VertexArrayName);

	return Validated;
}

void display()
{
	// Update of the uniform buffer
	{
		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		glm::mat4* Pointer = (glm::mat4*)glMapBufferRange(
			GL_UNIFORM_BUFFER, 0,	sizeof(glm::mat4),
			GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

		glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
		glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -Window.TranlationCurrent.y));
		glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, Window.RotationCurrent.y, glm::vec3(1.f, 0.f, 0.f));
		glm::mat4 View = glm::rotate(ViewRotateX, Window.RotationCurrent.x, glm::vec3(0.f, 1.f, 0.f));
		glm::mat4 Model = glm::mat4(1.0f);
		
		*Pointer = Projection * View * Model;

		// Make sure the uniform buffer is uploaded
		glUnmapBuffer(GL_UNIFORM_BUFFER);
	}

	glViewportIndexedf(0, 0, 0, GLfloat(Window.Size.x), GLfloat(Window.Size.y));

	float DepthValue(1.0f);
	glClearBufferfv(GL_DEPTH, 0, &DepthValue);
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);

	// Bind rendering objects
	glBindProgramPipeline(PipelineName);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureName[texture::DIFFUSE]);
	glBindImageTexture(glf::semantic::image::PICKING, TextureName[texture::PICKING], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glBindVertexArray(VertexArrayName);
	glBindBufferBase(GL_UNIFORM_BUFFER, glf::semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);

	glDrawElementsInstancedBaseVertexBaseInstance(
		GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, 0, 5, 0, 0);

	glf::swapBuffers();

	glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::PICKING]);
	float* Pointer = (float*)glMapBufferRange(GL_ARRAY_BUFFER, 0,	sizeof(float), GL_MAP_READ_BIT);
	float Depth = *Pointer;
	glUnmapBuffer(GL_ARRAY_BUFFER);

	printf("Depth: %2.3f\n", Depth);
}

int main(int argc, char* argv[])
{
	return glf::run(
		argc, argv,
		glm::ivec2(::SAMPLE_SIZE_WIDTH, ::SAMPLE_SIZE_HEIGHT), 
		WGL_CONTEXT_CORE_PROFILE_BIT_ARB, 
		::SAMPLE_MAJOR_VERSION, ::SAMPLE_MINOR_VERSION);
}
