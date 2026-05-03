#include <list>
#include <vector>
#include <algorithm>

#include "point.h"

point evaluate(float t, std::list<point> P)
{
	
	std::list<point> Q(P);

	while (Q.size() > 1) {

		std::list<point> R;

		std::list<point>::iterator P1 = Q.begin();
		std::list<point>::iterator P2 = Q.begin();
		std::advance(P2, 1);

		while (P2 != Q.end()) {

			point p = ((1 - t) * *P1) + (t * *P2);
			R.push_back(p);
			
			std::advance(P1, 1);
			std::advance(P2, 1);
		}
		Q.clear();
		Q = R;
	}
	
	
	return Q.front();
}

std::vector<point> EvaluateBezierCurve(std::vector<point>ctrl_points, int num_evaluations)
{
	std::list<point> ps(ctrl_points.begin(), ctrl_points.end()); // copy vector of control points arg to a list
	std::vector<point> curve; // vector of points to return

	float offset = 1.f / num_evaluations;
	
	curve.push_back(ctrl_points.front());

	for (int e = 0; e < num_evaluations; e++) {

		point p = evaluate(offset * (e + 1), ps);
		curve.push_back(p);
	}

	return curve;
}

float* MakeFloatsFromVector(std::vector<point> curve, int& num_verts, int& num_floats, float r, float g, float b)
{
	num_verts = curve.size();

	if (num_verts == 0) {
		return NULL;
	}
	
	num_floats = num_verts * 6;
	
	float* vertices = new float[num_floats];

	int i = 0;
	for (point p: curve) {
		vertices[i] = p.x;
		vertices[i + 1] = p.y;
		vertices[i + 2] = p.z;

		vertices[i + 3] = r;
		vertices[i + 4] = g;
		vertices[i + 5] = b;

		i += 6;
	}

	return vertices;
}
