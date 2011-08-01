#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>		// Header File For Windows

#include <gl\gl.h>		// Header File For The Glaux Library
#include <gl\glaux.h>		// Header File For The Glaux Library

#include <vector>

/** Vertex class
	@descrip
		Nothing fancy here, just a simple 3D point with support
		for some basic operators.
*/
class vertex {
public:

	///Default constructor
	vertex():x(0),y(0),z(0){}

	///Constructor
	vertex(float ax,float ay,float az):x(ax),y(ay),z(az){}
	
	///Copy constructor
	vertex(const vertex&p){*this = p;}
	
	//Destructor
	~vertex(){}

	///Divide by constant
	vertex operator/(const float &f){
		vertex ret;

		ret.x = x/f;
		ret.y = y/f;
		ret.z = z/f;

		return ret;
	}

	///Mult. by constant
	vertex operator*(const float &f){
		vertex ret;

		ret.x = x*f;
		ret.y = y*f;
		ret.z = z*f;

		return ret;
	}

	///Subtract vertex
	vertex operator-(const vertex &p){
		vertex ret;

		ret.x = x - p.x;
		ret.y = y - p.y;
		ret.z = z - p.z;

		return ret;
	}

	///Add vertex
	vertex operator+(const vertex &p){
		vertex ret;

		ret.x = x + p.x;
		ret.y = y + p.y;
		ret.z = z + p.z;

		return ret;
	}

	///Copy vertex
	void operator=(const vertex &p){
		x = p.x;
		y = p.y;
		z = p.z;
	}

	///Coordinates
	float x,y,z;
};

/*	Vertex track class
	@descrip
		Vertex tracks are vertexs with direction and a timer.  Vertices
		can be delayed(offset) to give the illusion of real 3D data
		separation (useful for simulating the "snow fall" effect)
*/
class vTrack {
public:
	///Default Constructor
	vTrack():base(vertex(0,0,0)),time_base(0.0f),time_max(0.0f),delay(0.0f),color(vertex(0.5,0.5,0.5)){
		dir = vertex(0,-1,0);
		original = base;
	}
	///vertex constructor
	vTrack(const vertex &ab):base(ab),time_base(0.0f),time_max(0.0f),delay(0.0f),color(vertex(0.5,0.5,0.5)){
		dir = vertex(0,-1,0);
		original = base;
	}
	///Copy constructor
	vTrack(const vTrack &v){
		*this = v;
	}
	///vertex/Dir constructor
	vTrack(const vertex &ab,const vertex &ad):base(ab),dir(ad){original = base;}
	
	///Destructor
	~vTrack(){}


	///Equals operator
	void operator=(const vTrack &v){
		original = v.original;
		base = v.base;
		dir = v.dir;
		color = v.color;
		time_base = v.time_base;
		delay = v.delay;
		time_max = v.time_max;
	}

	///Set vertex
	void set_vertex(const float &dt){
		float time = dt;
		base = base + (dir * time);
	}

	/*	Get vertex next time step
		@notes
			Get vertex increments time_base by some time step.  Return vertex
			is never to exceed time_max.
	*/
	vertex get_vertex(const float &dt){
		vertex ret;
		delay -= dt;

		///Only update if delay has reached 0
		if(delay <= 0)time_base += dt;

		///Cap time step by time_max
		if(time_base > time_max)time_base = time_max;
		
		ret = base + (dir * time_base);
		return ret;
	}

	///Max total time allowed per vertex
	float time_max;
	
	///Timer delay
	float delay;
	
	///Starting time step
	float time_base;

	///Color (not really a vertex)
	vertex color;

	///Direction
	vertex dir;

	///Base vertex
	vertex base;
	
	///Original vertex(unused)
	vertex original;
};

///Sort data by height in a particular direction
void	sort_by_height(int dir=1);

///Update scene
int		Update(HDC hDC);

///Init OpenGL
BOOL	Init(HWND hwnd, HDC &hDC, HGLRC &hRC);

///Kill OpenGL
GLvoid	Kill(HWND hwnd, HDC hDC, HGLRC hRC);

///Add color to color array (NOTE:  Vertex array must be filled)
void add_color(int idx, float x,float y,float z);

///Add vertex to vertex array
void add_vertex(float x,float y,float z);

///Normalize data to the range 0-1 centered at origin( vertex(0,0,0) ).
void normalize_vertices();