#version 450

layout (push_constant) uniform FragConsts
{
    vec4 u_color;
};

layout (location = 0) out vec4 f_color;

void main()
{
    f_color = u_color;
}
