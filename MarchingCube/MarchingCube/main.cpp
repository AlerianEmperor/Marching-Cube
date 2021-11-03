#include <Windows.h>
#include "vec3.h"
#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "GL\GLU.H"
#include "GL\glut.h"
#include "GL\GL.H"
#include "LookupTable.h"
#include <math.h>

#define kWindowWidth 800
#define kWindowHeight 600

using namespace std;
/*
struct Triangle
{
	Triangle(){}
	Triangle(int v0, int v1, int v2)
	{
		v[0] = v0;
		v[1] = v1;
		v[2] = v2;
	}
	Triangle(int v0, int v1, int v2, vec3 n_)
	{
		v[0] = v0;
		v[1] = v1;
		v[2] = v2;
		n = n_;
	}
	 
	int v[3];
	vec3 n;
};
*/

struct Triangle
{
	Triangle() {}
	Triangle(vec3 p1_, vec3 p2_, vec3 p3_) : p1(p1_), p2(p2_), p3(p3_) {}
	Triangle(vec3 p1_, vec3 p2_, vec3 p3_, vec3 n_) : p1(p1_), p2(p2_), p3(p3_), n(n_) {}
	Triangle(vec3 p1_, vec3 p2_, vec3 p3_, vec3 n_, vec3 n2_, vec3 n3_) : p1(p1_), p2(p2_), p3(p3_), n(n_), n2(n2_), n3(n3_) {}
	vec3 p1;
	vec3 p2;
	vec3 p3;
	vec3 n;
	vec3 n2;
	vec3 n3;
};

//Global Variables
GLfloat r, ang, dang, rot = 0, drot = 1.45f;


GLfloat lightAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat lightSpecular[] = { 0.2f, 0.2f, 0.2f, 1.0f };
GLfloat lightPosition[] = { 50.0f, -40.0f, -20.0f, 1.0f };
GLfloat materialAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat materialDiffuse[] = { 0.5f, 0.6f, 1.0f, 1.0f };
GLfloat materialSpecular[] = { 0.1f, 0.1f, 0.1f, 1.0f };

static float tangle_cube(const float& x, const float& y, const float& z)
{
	float x2 = x * x;
	float y2 = y * y;
	float z2 = z * z;

	return x2 * x2 + y2 * y2 + z2 * z2 - 5 * (x2 + y2 + z2) + 11.8f;
}

static float tangle_cube(const vec3& v)
{
	float x = v.x, y = v.y, z = v.z;
	float x2 = x * x;
	float y2 = y * y;
	float z2 = z * z;

	
	return x2 * x2 + y2 * y2 + z2 * z2 - 5 * (x2 + y2 + z2) + 11.8f;

	//return -y2 * y2 - x2 * x2 - z2 * z2 + 2 * x2 * z2 + 4 * y2 - 2 * x2 * y2 - 2 * y2 * z2;
	//return 64 * x2*x2*x2*x2 - 128 * x2*x2*x2 + 80 * x2*x2 - 16 * x2 + 2 + 64 * y2*y2*y2*y2 - 128 * y2*y2*y2 + 80 * y2*y2 - 16 * y2 + 64 * z2*z2*z2*z2 - 128 * z2*z2*z2 + 80 * z2*z2 - 16 * z2;
}
/*
float bounding_size = 2.0f;
int grid_size = 36;
float half_bounding_size;
float sub_grid_size;
float half_sub_grid_size;
float iso_level = 3.0f;
*/

float bounding_size = 16.0f;
int grid_size = 95;
float half_bounding_size = bounding_size / 2.0f;
float sub_grid_size = bounding_size / grid_size;
float half_sub_grid_size = sub_grid_size / 2.0f;
float iso_level = 3.0f;

/*bounding_size = 6.0f;
half_bounding_size = bounding_size / 2.0f;
sub_grid_size = bounding_size / grid_size;
half_sub_grid_size = sub_grid_size / 2.0f;*/

/*static void find_scale_value()
{
	float scale = 1.0f;
	half_bounding_size = bounding_size * 0.5f;
	vec3 upper_bound(half_bounding_size);
	vec3 lower_bound(-half_bounding_size);
	//khi be mat cua hinh hoc dang xet nam ben ngoai pham vi to nhat
	//ta scale diem p len cho den khi no bao phu duoc toan bo be mat dang xet
	while (tangle_cube(upper_bound) < 0.0f || tangle_cube(lower_bound) < 0.0f)
	{
		upper_bound *= 1.05f;
		lower_bound *= 1.05f;
		scale *= 1.05f;
	}
	
	
	bounding_size *= scale;
	half_bounding_size = bounding_size / 2.0f;
	sub_grid_size = bounding_size / grid_size;
	half_sub_grid_size = sub_grid_size / 2.0f;
	

	bounding_size = 6.0f;
	half_bounding_size = bounding_size / 2.0f;
	sub_grid_size = bounding_size / grid_size;
	half_sub_grid_size = sub_grid_size / 2.0f;
}*/

//
//                z
//                |
//                |
//                |
//                |
//				   -------------- x
//               /
//              /
//             /
//            y
vec3 edge_direction[8] =
{
	vec3(0, 0, 0), vec3(1, 0, 0), vec3(1, 1, 0), vec3(0, 1, 0),
	vec3(0, 0, 1), vec3(1, 0, 1), vec3(1, 1, 1), vec3(0, 1, 1)
};

int edge_point[12][2] =
{
	{0, 1}, {1, 2}, {2, 3}, {3, 0},
	{4, 5}, {5, 6}, {6, 7}, {7, 4},
	{0, 4}, {1, 5}, {2, 6}, {3, 7}
};




static void compute_grid(vector<Triangle>& triangles, vec3 cube_vertex[8])
{
	
	//lowest 
	//-a, -a, -a
	
	//highest 
	//a, a, a
	
	//center of cube have coordinate (x, y, z)
	//2 * a / grid_size + x * sub_grid_size.x + y * sub_grid_size * y + z * sub_gid_size.z
	//vec3(-a, -a, -a) + vec3(2 * a / grid_size) + (x * sub_grid_size.x, y * sub_grid_size.y + z * sub_grid_size.z)

	vec3 lowest_point(-half_bounding_size);
	vec3 offset(half_sub_grid_size);
	//cout << half_sub_grid_size << "\n";
	//cout << half_bounding_size << "\n";
	for (int x = 0; x < grid_size; ++x)
	{
		for (int y = 0; y < grid_size; ++y)
		{
			for (int z = 0; z < grid_size; ++z)
			{
				int cube_index = 0;
				vec3 center(lowest_point + offset + vec3(x  * sub_grid_size, y  * sub_grid_size, z  * sub_grid_size));
				//cout << center.x << " " << center.y << " " << center.z << "\n";
				float vertex_value[8];
				vec3 vertex_position[8];
				for (int i = 0; i < 8; ++i)
				{
					vec3 p = center + cube_vertex[i];
					//cout << p.x << " " << p.y << " " << p.z << "\n";
					//cout << cube_vertex[i].x << " " << cube_vertex[i].y << " " << cube_vertex[i].z << "\n";
					float v = tangle_cube(p);
					if (v <= iso_level)//interesting bugs
					{
						cube_index |= 1 << i;
					}
					vertex_value[i] = abs(v);
					vertex_position[i] = p;
				}

				if (cube_index == 0 || cube_index == 255)
				{
					continue;
				}
				else
				{
					int edge_index = EdgeTable[cube_index];

					int cutted_edge[12] = { -1 };
					vec3 cutted_points[12];
					int index = 0;
					while (edge_index)
					{
						if (edge_index & 1)
							cutted_edge[index] = 1;
						++index;
						edge_index >>= 1;
					}

					for (int i = 0; i < 12; ++i)
					{
						if (cutted_edge[i] != -1)
						{
							int i1 = edge_point[i][0];
							int i2 = edge_point[i][1];

							vec3 p1(vertex_position[i1]);
							vec3 p2(vertex_position[i2]);

							//need interpolation
							//cutted_points[i] = p1 + (p2 - p1) * vertex_value[i1] / (vertex_value[i1] + vertex_value[i2]);
							//cutted_points[i] = p1 + (p2 - p1) * vertex_value[i1] / (vertex_value[i2] - vertex_value[i1]);

							float v1 = vertex_value[i1];
							float v2 = vertex_value[i2];

							if (v1 < 0.000001f)
								cutted_points[i] = p1;
							else if (v2 < 0.0000001f)
								cutted_points[i] = p2;
							else if(abs(v1 - v2) < 0.000001f)
							{
								cutted_points[i] = p1;
							}
							else
							{
								//float mu = -v1 / (v2 - v1);
								//cutted_points[i] = p1 + mu *(p2 - p1);
								cutted_points[i] = p1 + (p2 - p1) * (iso_level-vertex_value[i1]) / (vertex_value[i2] - vertex_value[i1]);
							}
						}
					}

					//Draw Triangles
					for (int i = 0; i < 16; i += 3)
					{
						if (TriangleTable[cube_index][i] == -1)
							break;
						int i1 = TriangleTable[cube_index][i];
						int i2 = TriangleTable[cube_index][i + 1];
						int i3 = TriangleTable[cube_index][i + 2];

						vec3 p1 = cutted_points[i1];
						vec3 p2 = cutted_points[i2];
						vec3 p3 = cutted_points[i3];

						vec3 n((p2 - p1).cross(p3 - p1).norm());
						triangles.emplace_back(p1, p2, p3, n);
					}
				}
			}
		}
	}

}

static void compute_grid_interpolate(vector<Triangle>& triangles, vec3 cube_vertex[8])
{

	//lowest 
	//-a, -a, -a

	//highest 
	//a, a, a

	//center of cube have coordinate (x, y, z)
	//2 * a / grid_size + x * sub_grid_size.x + y * sub_grid_size * y + z * sub_gid_size.z
	//vec3(-a, -a, -a) + vec3(2 * a / grid_size) + (x * sub_grid_size.x, y * sub_grid_size.y + z * sub_grid_size.z)

	vec3 lowest_point(-half_bounding_size);
	vec3 offset(half_sub_grid_size);
	//cout << half_sub_grid_size << "\n";
	//cout << half_bounding_size << "\n";
	for (int x = 0; x < grid_size; ++x)
	{
		for (int y = 0; y < grid_size; ++y)
		{
			for (int z = 0; z < grid_size; ++z)
			{
				int cube_index = 0;
				vec3 center(lowest_point + offset + vec3(x * 1.0f * sub_grid_size, y * 1.0f * sub_grid_size, z * 1.0f * sub_grid_size));
				//cout << center.x << " " << center.y << " " << center.z << "\n";
				float vertex_value[8];
				vec3 vertex_position[8];
				for (int i = 0; i < 8; ++i)
				{
					vec3 p = center + cube_vertex[i];
					//cout << p.x << " " << p.y << " " << p.z << "\n";
					//cout << cube_vertex[i].x << " " << cube_vertex[i].y << " " << cube_vertex[i].z << "\n";
					float v = tangle_cube(p);
					if (v <= iso_level)//interesting bugs
					{
						cube_index |= 1 << i;
					}
					vertex_value[i] = v;
					vertex_position[i] = p;
				}

				if (cube_index == 0 || cube_index == 255)
				{
					continue;
				}
				else
				{
					int edge_index = EdgeTable[cube_index];

					int cutted_edge[12] = { -1 };
					vec3 cutted_points[12] = { vec3(inf) };
					int index = 0;
					while (edge_index)
					{
						if (edge_index && 1)
							cutted_edge[index] = 1;
						++index;
						edge_index >>= 1;
					}

					for (int i = 0; i < 12; ++i)
					{
						if (cutted_edge[i] != -1)
						{
							int i1 = edge_point[i][0];
							int i2 = edge_point[i][1];

							vec3 p1(vertex_position[i1]);
							vec3 p2(vertex_position[i2]);
					
							float v1 = vertex_value[i1];
							float v2 = vertex_value[i2];
							/*
							if (abs(iso_level - v1) < 0.00001)
							{
								cutted_points[i] = p1;
							}
							else if (abs(iso_level - v2) < 0.00001)
							{
								cutted_points[i] = p2;
							}
							else if (abs(v1 - v2) < 0.00001)
							{
								cutted_points[i] = p1;
							}
							else
							{
								float m = (iso_level - v1) / (v1 - v2);
								cutted_points[i] = p1 + m * (p2 - p1);
							}
							*/
							//Good
							if (p1 < p2)
							{
								swap(p1, p2);
								swap(v1, v2);
							}
							vec3 p;
							if (abs(v1 - v2) > 0.00001)
							{
								p = p1 + (p2 - p1) * (iso_level - v1) / (v2 - v1);
								
								//p = p1 + (p2 - p1) * v1 / (v2 + v1);

								//bad
								//p = p1 + (p2 - p1) * abs(v1 / (v2 - v1));

								//bad
								//p = p1 + (p2 - p1) * abs(v1 / (v1 + v2));
							}
							else
								p = p1;
							cutted_points[i] = p;
							

							//not as good
							/*
							if (v1 < 0.00001f)
								cutted_points[i] = p1;
							else if (v2 < 0.0000001f)
								cutted_points[i] = p2;
							else if (abs(v1 - v2) < 0.00001f)
							{
								cutted_points[i] = p1;
							}
							else
							{
								//float mu = -v1 / (v2 - v1);
								//cutted_points[i] = p1 + mu *(p2 - p1);
								cutted_points[i] = p1 + (p2 - p1) * (iso_level - vertex_value[i1]) / (vertex_value[i2] - vertex_value[i1]);
							}
							*/
						}
					}

					//Draw Triangles
					for (int i = 0; i < 16; i += 3)
					{
						if (TriangleTable[cube_index][i] == -1 || TriangleTable[cube_index][i + 1] == -1 || TriangleTable[cube_index][i + 2] == -1)
							break;
						int i1 = TriangleTable[cube_index][i];
						int i2 = TriangleTable[cube_index][i + 1];
						int i3 = TriangleTable[cube_index][i + 2];


						vec3 p1 = cutted_points[i1];
						vec3 p2 = cutted_points[i2];
						vec3 p3 = cutted_points[i3];
						/*vec3 p1, p2, p3;
						if (cutted_points[i1].x != inf)
							p1 = cutted_points[i1];

						if (cutted_points[i2].x != inf)
							p2 = cutted_points[i2];

						if (cutted_points[i3].x != inf)
							p3 = cutted_points[i3];*/

						vec3 n((p1 - p2).cross(p1 - p3).norm());
						vec3 n2((p2 - p3).cross(p2 - p1).norm());
						vec3 n3((p3 - p1).cross(p3 - p2).norm());
						n = (n + n2 + n3) / 3.0f;
						triangles.emplace_back(p1, p2, p3, n);
					}
				}
			}
		}
	}

	/*vec3 view_direction(0.0f, -0.5f, 0.0f);

	for (auto& v : triangles)
	{
		if (view_direction.dot(v.n) > 0)
			v.n = -v.n;
	}*/
	
	/*int s = triangles.size();
	vec3 view_direction(0.0f, 0.0f, -0.5f);
	for (int i = 0; i < s; ++i)
	{
		if (view_direction.dot(triangles[i].n) >= 0)
			triangles[i].n = -triangles[i].n;
	}*/
}
vector<Triangle> triangles;
GLvoid InitGL(GLvoid)
{
	glClearColor(0.1f, 0.2f, 0.5f, 0.0f);
	glClearDepth(1.0);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glShadeModel(GL_SMOOTH);

	glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmbient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, lightSpecular);
	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluLookAt(0.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 1.0, 0.0);
	gluPerspective(45.0f, (GLfloat)kWindowWidth / (GLfloat)kWindowHeight, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);

	//scale cube to fit geometry
	//find_scale_value();

	vec3 cube_vertex[8] =
	{
		vec3(-half_sub_grid_size, -half_sub_grid_size, -half_sub_grid_size),
		vec3(half_sub_grid_size, -half_sub_grid_size, -half_sub_grid_size),
		vec3(half_sub_grid_size, -half_sub_grid_size,  half_sub_grid_size),
		vec3(-half_sub_grid_size, -half_sub_grid_size,  half_sub_grid_size),
		vec3(-half_sub_grid_size,  half_sub_grid_size, -half_sub_grid_size),
		vec3(half_sub_grid_size,  half_sub_grid_size, -half_sub_grid_size),
		vec3(half_sub_grid_size,  half_sub_grid_size,  half_sub_grid_size),
		vec3(-half_sub_grid_size,  half_sub_grid_size,  half_sub_grid_size)
	};

	compute_grid_interpolate(triangles, cube_vertex);
}

GLvoid ReSizeGLScene(int Width, int Height)
{
	glViewport(0, 0, (GLsizei)Width, (GLsizei)Height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//float pov = 1.0f / (2.0f * bounding_size) * 180.0f;

	gluPerspective(45.0f, (GLfloat)Width / (GLfloat)Height, 0.1, 1000.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


GLvoid DrawGLScene(GLvoid)
{
	//cout << "i am running" << "\n";
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	//glShadeModel(GL_SMOOTH);
	//glEnable(GL_FRONT);
	//glFrontFace(GL_CCW);
	glTranslatef(0.0f, 0.0f, -10.0f);
	glRotatef(rot / 3, 1.0f, 0.0f, 0.0f);
	glRotatef(rot / 3, 0.0f, 1.0f, 0.0f);
	glRotatef(rot / 3, 0.0f, 0.0f, 1.0f);

	//cout << triangles.size() << "\n";
	
	//int s = triangles.size();
	//cout << s << "\n";
	glBegin(GL_TRIANGLES);
	//glColor3f(1.0f, 1.0f, 1.0f);
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
	
	/*for (int i = 0; i < s; ++i)
	{
		glNormal3f(triangles[i].n.x, triangles[i].n.y, triangles[i].n.z);
		glVertex3f(triangles[i].p1.x, triangles[i].p1.y, triangles[i].p1.z);
		glVertex3f(triangles[i].p2.x, triangles[i].p2.y, triangles[i].p2.z);
		glVertex3f(triangles[i].p3.x, triangles[i].p3.y, triangles[i].p3.z);
	}*/

	for (auto& v : triangles)
	{
		glNormal3f(v.n.x, v.n.y, v.n.z);
		glVertex3f(v.p1.x, v.p1.y, v.p1.z);
		//glNormal3f(v.n2.x, v.n2.y, v.n2.z);
		glVertex3f(v.p2.x, v.p2.y, v.p2.z);
		//glNormal3f(v.n3.x, v.n3.y, v.n3.z);
		glVertex3f(v.p3.x, v.p3.y, v.p3.z);
	}

	rot += drot;//0.25;

	glEnd();

	glFlush();
	glutSwapBuffers();
	glutPostRedisplay();
}

void Keypress(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		exit(0);
	case 32:
		if (drot == 0.25f)
			drot = 0.0f;
		else drot = 0.25f;
	
	}
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(kWindowWidth, kWindowHeight);
	glutInitWindowPosition(100, 100);
	glutCreateWindow(argv[0]);

	InitGL();

	glutDisplayFunc(DrawGLScene);
	glutReshapeFunc(ReSizeGLScene);
	glutKeyboardFunc(Keypress);
	glutMainLoop();


	return 0;
}



static void draw_grid(vector<Triangle>& triangles, const int& grid_size)
{
	float sub_grid_size = 1.0f / grid_size;
	//vec3 lower_bound(-bounding_size);

	//triangles.reserve(grid_size * grid_size * grid_size * 5);
	for (int x = 0; x < grid_size; ++x)
	{
		for (int y = 0; y < grid_size; ++y)
		{
			for (int z = 0; z < grid_size; ++z)
			{
				//int sum = x + y + z;
				//lower_bound = vec3(x * 1.0f, y * 1.0f, z * 1.0f) * sub_grid_size;
				//cout <<"vertex: "<< lower_bound.x << " " << lower_bound.y << " " << lower_bound.z << "\n";

				int cube_index = 0;
				float vertex_value[8];
				vec3 vertex_position[8];
				for (int i = 0; i < 8; ++i)
				{
					//vec3 p = lower_bound + edge_direction[vertex] * sub_grid_size * (x + y + z);
					vec3 p = vec3(x, y, z) * edge_direction[i] * sub_grid_size;// *sum;

					float v = tangle_cube(p);
					if (v < 0)
					{
						cube_index |= 1 << i;
					}
					vertex_value[i] = abs(v);
					vertex_position[8] = p;
				}
				if (cube_index == 0 || cube_index == 255)
					continue;
				else
				{
					//find cutted edge use Edge Table
					int cut_edges[12] = { 0 };
					int edge_index = EdgeTable[cube_index];
					int ei = 0;
					while (edge_index)
					{
						if (edge_index & 1 << ei)
							cut_edges[ei] = 1;
						edge_index >>= 1;
						++ei;
					}

					//find points of intersecion
					vec3 cut_points[12] = { inf };

					for (int i = 0; i < 12; ++i)
					{
						if (cut_edges[i])
						{
							int p1 = edge_point[i][0];
							int p2 = edge_point[i][1];

							//need interpolation
							cut_points[i] =
								vertex_position[p1] + (vertex_position[p2] - vertex_position[p1])
								* (vertex_value[p1]) / (vertex_value[p1] + vertex_value[p2]);

						}
					}

					//find triangle that connect points
					//int triangle_index[16];// = TriangleTable[cube_index];
					//for(int i)
					for (int i = 0; i < 16; i += 3)
					{
						if (TriangleTable[cube_index][i] == -1)
							break;
						int t1 = TriangleTable[cube_index][i];
						int t2 = TriangleTable[cube_index][i + 1];
						int t3 = TriangleTable[cube_index][i + 2];

						vec3 n((cut_points[t2] - cut_points[t1]).cross(cut_points[t3] - cut_points[t1]).norm());

						triangles.emplace_back(Triangle(cut_points[t1], cut_points[t2], cut_points[t3], n));
					}
				}
			}
		}
	}
	/*int s = triangles.size();
	std::cout << s << "\n";
	glBegin(GL_TRIANGLES);
	glColor3f(1.0f, 1.0f, 1.0f);
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);

	for (int i = 0; i < s; ++i)
	{
	glNormal3f(triangles[i].n.x, triangles[i].n.y, triangles[i].n.z);
	glVertex3f(triangles[i].p1.x, triangles[i].p1.y, triangles[i].p1.z);
	glVertex3f(triangles[i].p2.x, triangles[i].p2.y, triangles[i].p2.z);
	glVertex3f(triangles[i].p3.x, triangles[i].p3.y, triangles[i].p3.z);
	}

	rot += drot;//0.25;
	glEnd();

	glFlush();
	glutSwapBuffers();
	glutPostRedisplay();*/
}

//vector<Triangle> triangles;