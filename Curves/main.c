//
//  main.c
//  Curves
//
//  Created by Steven Rossum on 02-05-12.
//  Copyright (c) 2012 Edwin Westerhoud and Steven van Rossum. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#if defined(__APPLE__) || defined(MACOSX)
#include <GLUT/GLUT.h>
#else
#include <GL/glut.h>
#endif

#include "List.h"

int order = 0;
int count = 0;
struct list * selected = 0;
int bezier = 0;
int uniform = 0;
float * knot = 0;
struct list * l = 0;

// GLUT callbacks.
void mouse(int button, int state, int x, int y);
void keyboard(unsigned char key, int x, int y);
void motion(int x, int y);
void display(void);
void reshape(int w, int h);

// Bezier computation.
void computeNOrderBezierlf(int n, struct list * l, float t, float * resx, float * resy);
int coeff(int n, int k);

// B-spline computation.
void computeNOrderBsplinelf(int d, struct list * l, float u, float * t, float * resx, float * resy);
float BsplineBasis(int j, int d, float u, float * t);
float * computeUniformKnot(int d, int n);
float * computeNonUniformKnot(int d, int n);

// Helper functions.
struct list * listGetNearestNodegl(struct list * l, int x, int y);
void printHelpglut();

// Main asks for initial input from the user and instructs the user how to interact at run-time.
int main (int argc, const char * argv[])
{   
	int gwind = 0;

	printf("While working:\n - Use 1 - 9 to change the degree.\n - Use 0 to change between bezier and bspline curves.\n - Use C to clear all the points.\n - Use U to change between uniform and nonuniform B-splines.\n - Use ESC to quit the program.\n - Left click with the mouse to add a control point.\n - Left drag with the mouse to move a control point.\n - Right click with the mouse to remove a control point.\n");
	while (order <= 0)
	{
		printf("\nPolynomial degree must be at least 1.\nPolynomial degree: ");
		if (!scanf("%d", &order)) fflush(stdin);
	}

    knot = computeNonUniformKnot(order + 1, count - 1);
    
    glutInit(&argc, (char **)argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowSize(500, 500);
    gwind = glutCreateWindow("N-th order Bezier and B-spline Curves by Edwin Westerhoud and Steven van Rossum");
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(1.0, 1.0, 1.0, 1.0);
	glPointSize(8.0f);
	glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);
    glutMotionFunc(motion);
    glutMainLoop();
    return 0;
}

// Returns the first node in a list that is within range of GL_POINT_SIZE.
struct list * listGetNearestNodegl(struct list * l, int x, int y)
{
    float pointSize;
    
    glGetFloatv(GL_POINT_SIZE, &pointSize);
    pointSize /= 2.0f;
    
    while (l)
    {
        if (l->x - pointSize <= x && l->x + pointSize >= x && l->y - pointSize <= y && l->y + pointSize >= y)
            return l;
        l = l->next;
    }
    
    return 0;
}

// Prints help on screen.
void printHelpglut()
{
    char s[100];
    char * st = 0;
    
    glColor3ub(0, 0, 0);
    glRasterPos2i(2, 12);
    sprintf(s, "%s curve of degree %i, with %i control points.", bezier ? "Bezier" : uniform ? "Uniform B-spline" : "Nonuniform B-spline", order, count);
    st = s;
    while (*st)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *st);
        st++;
    }
    
    glRasterPos2i(2, 36);
    st = "[1..9] - degree";
    while (*st)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *st);
        st++;
    }
    
    glRasterPos2i(2, 48);
    st = "[0] - bezier/b-spline";
    while (*st)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *st);
        st++;
    }
    
    glRasterPos2i(2, 60);
    st = "[U] - uniform/nonuniform";
    while (*st)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *st);
        st++;
    }
    
    glRasterPos2i(2, 72);
    st = "[C] - clear";
    while (*st)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *st);
        st++;
    }
    
    glRasterPos2i(2, 84);
    st = "[ESC] - quit";
    while (*st)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *st);
        st++;
    }
    
    glRasterPos2i(2, 96);
    st = "LMB - add, move";
    while (*st)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *st);
        st++;
    }
    
    glRasterPos2i(2, 108);
    st = "RMB - remove";
    while (*st)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *st);
        st++;
    }
}

// GLUT callback for mouse dragging.
void motion(int x, int y)
{
    if (selected)
    {
        selected->x = (float)x;
        selected->y = (float)y;
        glutPostRedisplay();
    }
}

// GLUT callback for mouse activity.
void mouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        if (!(selected = listGetNearestNodegl(l, x, y)))
        {
            if (l)
                listAdd(l, (float)x, (float)y);
            else
                l = listNew((float)x, (float)y);
            
            count++;
            
            free(knot);
            
            if (uniform)
                knot = computeUniformKnot(order + 1, count - 1);
            else
                knot = computeNonUniformKnot(order + 1, count - 1);
        }
        
        glutPostRedisplay();
    }
    else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
    {
        if ((selected = listGetNearestNodegl(l, x, y)))
        {
            if (l == selected)
                l = listRemove(selected);
            else
                listRemoveAt(selected, 0);
            
            selected = 0;
            count --;
            
            free(knot);
            
            if (uniform)
                knot = computeUniformKnot(order + 1, count - 1);
            else
                knot = computeNonUniformKnot(order + 1, count - 1);
        }
        
        glutPostRedisplay();
    }
}

// GLUT callback for keyboard activity.
// 1 - 9 to change the degree of the polynomial.
// 0 to change between bezier and b-spline.
// C to clear the control points.
// ESC to quit.
// U to switch between uniform and nonuniform bspline curves.
void keyboard(unsigned char key, int x, int y)
{
	if (key >= '1' && key <= '9')
        order = (int)key - 48;
    else if (key == '0')
        bezier = !bezier;
	else if (key == 'c' || key == 'C')
	{
		selected = 0;
		listFree(l);
		l = 0;
        count = 0;
	}
	else if ((int)key == 27)
	{
		glutDestroyWindow(glutGetWindow());
		exit(0);
	}
	else if ((key == 'u' || key == 'U') && !bezier)
		uniform = !uniform;
    
    if (!bezier)
    {
        free(knot);
        
        if (uniform)
            knot = computeUniformKnot(order + 1, count - 1);
        else
            knot = computeNonUniformKnot(order + 1, count - 1);
    }
	glutPostRedisplay();
	return;
}

// GLUT callback for rendering.
// Draws all the control points in l, their connecting lines and the curve as specified by the user.
// Ends by printing a description of the current curve to the screen.
void display()
{
	struct list * tmp = l;
	float x, y;
	float t;

    glClear(GL_COLOR_BUFFER_BIT);
    glColor3ub(127, 127, 127);
	glBegin(GL_LINE_STRIP);

	tmp = l;
	while (tmp)
    {
        glVertex2f(tmp->x, tmp->y);
        tmp = tmp->next;
    }

	glEnd();

    glColor3ub(255, 0, 0);

    glBegin(GL_POINTS);
	tmp = l;
    while (tmp)
    {
        glVertex2f(tmp->x, tmp->y);
        tmp = tmp->next;
    }
    glEnd();

    glColor3ub(0, 0, 0);
    glBegin(GL_LINE_STRIP);
	tmp = l;
    if (bezier)
    {
        while (tmp && listCount(tmp) > order)
        {
            for (t = 0.0f; t <= 1.0f; t+=0.01f)
            {
                computeNOrderBezierlf(order, tmp, t, &x, &y);
                glVertex2f(x, y);
            }
            tmp = listGet(tmp, order);
        }
    }
    else
    {
        for (t = knot[order]; t <= knot[count]; t+=0.01f)
        {
            computeNOrderBsplinelf(order + 1, l, t, knot, &x, &y);
            glVertex2f(x, y);
        }
    }
    glEnd();
    
    printHelpglut();
    
    glutSwapBuffers();
}

// GLUT callback for window resizing.
void reshape(int w, int h)
{
    glViewport(0, 0, w, h);		
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();    
    glOrtho( 0, w, h, 0, -1.0, 1.0 );
    glMatrixMode(GL_MODELVIEW);
}

// Computes a point on a N-th degree bezier curve at interval t.
// Returns the point through resx and resy.
void computeNOrderBezierlf(int n, struct list * l, float t, float * resx, float * resy)
{
    float omt = 1.0f - t;
	int i = 0;
	float c;
    *resx = 0.0f;
    *resy = 0.0f;
    while (l && i <= n)
    {
        c = (float)coeff(n, i);
        *resx += c * powf(omt, n - i) * powf(t, i) * l->x;
        *resy += c * powf(omt, n - i) * powf(t, i) * l->y;
        l = l->next;
        i++;
    }
}

// Recursively computes the binomial coefficients for Bezier curves.
int coeff(int n, int k)
{
    if (n == 0 && k > 0)
        return 0;
    if (k == 0)
        return 1;
    return coeff(n-1, k-1) + coeff(n-1, k);
    
}

// Computes all (m = d + n + 1) uniform knot vectors.
float * computeUniformKnot(int d, int n)
{
	int j = 0;
    int m = (d + n + 1);
	float * t = malloc(sizeof(float) * m);
    
    for (j = 0; j < m; j++)
		t[j] = (float)j;

	return t;
}

// Computes all (m = d + n + 1) nonuniform knot vectors.
float * computeNonUniformKnot(int d, int n)
{
	int j = 0;
    int m = (d + n + 1);
	float * t = malloc(sizeof(float) * m);

	for (j = 0; j < m; j++)
	{
		if (j < d)
			t[j] = 0.0f;
		else if (d <= j && j <= n)
			t[j] = (float)(j - d + 1);
		else if (j > n)
			t[j] = (float)(n - d + 2);
	}

	return t;
}

// Computes a point on a N-th degree (d - 1) B-spline curve at interval u.
// Returns the point through resx and resy.
void computeNOrderBsplinelf(int d, struct list * l, float u, float * t, float * resx, float * resy)
{
	int j = 0;
	
	*resx = 0.0f;
	*resy = 0.0f;
    
    j = 0;
    while (l)
    {
        *resx += l->x * BsplineBasis(j, d, u, t);
        *resy += l->y * BsplineBasis(j, d, u, t);
        l = l->next;
        j++;
    }
}

// Recursively computes the B-spline basis function at interval u.
// Handles division by zero as zero and stops recursing in a branch when multiplication by zero would occur.
float BsplineBasis(int j, int d, float u, float * t)
{
	if (d == 1)
		if (t[j] <= u && u < t[j+1]) return 1.0f; else return 0.0f;
	else
		return ((t[j+d-1] == t[j]) ? 0.0f : (u-t[j]) / (t[j+d-1]-t[j]) * BsplineBasis(j, d-1, u, t)) + ((t[j+d] == t[j+1]) ? 0.0f : (t[j+d]-u)/(t[j+d]-t[j+1]) * BsplineBasis(j+1, d-1, u, t));
}