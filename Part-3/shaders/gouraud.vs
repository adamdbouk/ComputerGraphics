//global variables from the CPU
uniform mat4 model;
uniform mat4 viewprojection;

//vars to pass to the pixel shader
varying vec3 v_wPos;
varying vec3 v_wNormal;

varying vec3 light_inten;

//here create uniforms for all the data we need here

uniform vec3 eye_pos;
uniform float alfa;

uniform vec3 mat_ambient;
uniform vec3 mat_diffuse;
uniform vec3 mat_specular;

uniform vec3 light_pos;
uniform vec3 light_ambient;
uniform vec3 light_diffuse;
uniform vec3 light_specular;

void main()
{	
	//convert local coordinate to world coordinates
	vec3 wPos = (model * vec4( gl_Vertex.xyz, 1.0)).xyz;
	//convert local normal to world coordinates
	vec3 wNormal = (model * vec4( gl_Normal.xyz, 0.0)).xyz;

	//pass them to the pixel shader interpolated
	v_wPos = wPos;
	v_wNormal = wNormal;

	//in GOURAUD compute the color here and pass it to the pixel shader
	vec3 ambient = mat_ambient * light_ambient;
	
	vec3 l = normalize( light_pos - wPos );
	vec3 v = normalize( eye_pos - wPos );
	vec3 aux = (wPos - light_pos);
	vec3 r = normalize( reflect(aux, wNormal) );
	
	float ln = dot(l, wNormal);
	ln = clamp(ln, 0, 1);
	vec3 diffuse = mat_diffuse * ln * light_diffuse;

	float rv = dot(r, v);
	rv = clamp( rv, 0, 1);
	rv = pow(rv, alfa);
	vec3 specular = mat_specular * rv * light_specular;

	light_inten = ambient + diffuse + specular;
	light_inten = clamp(light_inten, 0, 1);


	//project the vertex by the model view projection 
	gl_Position = viewprojection * vec4(wPos,1.0); //output of the vertex shader
}
