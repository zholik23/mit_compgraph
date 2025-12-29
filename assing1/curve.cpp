#include "curve.h"
#include "extra.h"
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include "vecmath.h"
using namespace std;

namespace
{
    // Approximately equal to.  We don't want to use == because of
    // precision issues with floating point.
    inline bool approx( const Vector3f& lhs, const Vector3f& rhs )
    {
        const float eps = 1e-8f;
        return ( lhs - rhs ).absSquared() < eps;
    }

    
}
    

Curve evalBezier( const vector< Vector3f >& P, unsigned steps )
{
    // Check
    if( P.size() < 4 || P.size() % 3 != 1 )
    {
        cerr << "evalBezier must be called with 3n+1 control points." << endl;
        exit( 0 );
    }

    vector< CurvePoint > result;
    
    // 1. Define the Bernstein Basis Matrix
    // This transforms the power basis [1, t, t^2, t^3] into Bernstein weights
    // Row 0: Coeffs for (1-t)^3 -> 1, -3, 3, -1
    Matrix4f M(
         1.0f, -3.0f,  3.0f, -1.0f,
         0.0f,  3.0f, -6.0f,  3.0f,
         0.0f,  0.0f,  3.0f, -3.0f,
         0.0f,  0.0f,  0.0f,  1.0f
    );

    // Track the previous binormal for the recursive update (TNB frame)
    // Arbitrary start as mentioned in assignment 
    Vector3f currentB = Vector3f(0, 1, 0); 

    // 2. Loop through each segment of the Bezier curve
    // A cubic Bezier segment uses 4 points. The next segment shares the last point.
    // So we jump by 3: (0,1,2,3), then (3,4,5,6), etc.
    for( unsigned i = 0; i < P.size() - 3; i += 3 )
    {
        // Get the 4 control points for this current segment
        Vector3f p0 = P[i];
        Vector3f p1 = P[i+1];
        Vector3f p2 = P[i+2];
        Vector3f p3 = P[i+3];

        // 3. Generate points for this segment
        // Note: To avoid duplicate points at the seams, you might want to start 
        // j at 1 for segments after the first one. 
        // However, standard implementations often just generate 0 to steps.
        for( unsigned j = 0; j <= steps; ++j )
        {
            // Avoid duplicating the point where segments join
            if (i > 0 && j == 0) continue; 

            float t = (float)j / (float)steps;

            // --- POSITION V ---
            // Power Basis
            Vector4f T_vec(1, t, t*t, t*t*t);
            // Bernstein Weights = M * T_vec
            Vector4f weights = M * T_vec;

            // Linear combination of control points
            Vector3f V = p0 * weights[0] + p1 * weights[1] + p2 * weights[2] + p3 * weights[3];

            // --- TANGENT T ---
            // Derivative of Power Basis: [0, 1, 2t, 3t^2]
            Vector4f T_deriv_vec(0, 1, 2*t, 3*t*t);
            Vector4f weights_deriv = M * T_deriv_vec;

            Vector3f T = p0 * weights_deriv[0] + p1 * weights_deriv[1] + p2 * weights_deriv[2] + p3 * weights_deriv[3];
            T.normalize();

            // --- NORMALS and BINORMALS ---
            // Recursive update logic from PDF Section 4.2 [cite: 132]
            Vector3f N, B;
            
            if (i == 0 && j == 0) {
                 // Initial frame case.
                 // Pick an arbitrary B that is not parallel to T.
                 // (0,0,1) or (0,1,0) usually works unless T is parallel to it.
                 Vector3f V_arbitrary = Vector3f(0, 0, 1);
                 if (abs(Vector3f::dot(T, V_arbitrary)) > 0.9f) {
                     V_arbitrary = Vector3f(0, 1, 0);
                 }
                 N = Vector3f::cross(V_arbitrary, T).normalized(); // N = B_0 x T (rough approx)
                 B = Vector3f::cross(T, N).normalized();           // B = T x N
            } else {
                 // Recursive update: N_i = B_{i-1} x T_i
                 N = Vector3f::cross(currentB, T).normalized();
                 // B_i = T_i x N_i
                 B = Vector3f::cross(T, N).normalized();
            }

            // Update state for next iteration
            currentB = B;

            // CORRECT WAY TO CREATE CURVEPOINT
            CurvePoint cp;
            cp.V = V;
            cp.T = T;
            cp.N = N;
            cp.B = B;
            result.push_back(cp);
        }
    }

    return result;
}

Curve evalBspline( const vector< Vector3f >& P, unsigned steps )
{
    // Check
    if( P.size() < 4 )
    {
        cerr << "evalBspline must be called with 4 or more control points." << endl;
        exit( 0 );
    }

    // 1. Define the Basis Matrices
    // B-Spline Basis
    Matrix4f Mb(
        1.0f/6.0f, -3.0f/6.0f,  3.0f/6.0f, -1.0f/6.0f,
        4.0f/6.0f,  0.0f/6.0f, -6.0f/6.0f,  3.0f/6.0f,
        1.0f/6.0f,  3.0f/6.0f,  3.0f/6.0f, -3.0f/6.0f,
        0.0f/6.0f,  0.0f/6.0f,  0.0f/6.0f,  1.0f/6.0f
    );
    
    // Bezier Basis
    Matrix4f M(
         1.0f, -3.0f,  3.0f, -1.0f,
         0.0f,  3.0f, -6.0f,  3.0f,
         0.0f,  0.0f,  3.0f, -3.0f,
         0.0f,  0.0f,  0.0f,  1.0f
    );

    // 2. Compute the Basis Change Matrix
    // We want to map B-Spline Geometry (Gb) to Bezier Geometry (Gbz).
    // The relationship is: Gbz = Gb * (Mb * M_inverse)
    Matrix4f B = Mb * M.inverse();

    // 3. Accumulate ALL Bezier control points
    vector<Vector3f> bezierControlPoints;

    for (unsigned i = 0; i <= P.size() - 4; ++i)
    {
        // Construct the Geometry Matrix for the current B-Spline segment
        // We put the 4 points as COLUMNS of the matrix
        Vector3f p0 = P[i];
        Vector3f p1 = P[i+1];
        Vector3f p2 = P[i+2];
        Vector3f p3 = P[i+3];

        Matrix4f Gb(
            p0[0], p1[0], p2[0], p3[0],
            p0[1], p1[1], p2[1], p3[1],
            p0[2], p1[2], p2[2], p3[2],
            0,     0,     0,     0
        );

        // Calculate the new Bezier points for this segment
        // Gbz = Gb * B
        Matrix4f Gbz = Gb * B;

        // Extract the 4 Bezier points from the columns of Gbz
        Vector3f q0( Gbz(0,0), Gbz(1,0), Gbz(2,0) );
        Vector3f q1( Gbz(0,1), Gbz(1,1), Gbz(2,1) );
        Vector3f q2( Gbz(0,2), Gbz(1,2), Gbz(2,2) );
        Vector3f q3( Gbz(0,3), Gbz(1,3), Gbz(2,3) );

        // Add to our main list
        // If this is the first segment, add all 4 points.
        // For subsequent segments, the first point (q0) is exactly the same 
        // as the last point (q3) of the previous segment.
        // evalBezier expects 3n+1 points, so we only add q1, q2, q3.
        if (i == 0) {
            bezierControlPoints.push_back(q0);
            bezierControlPoints.push_back(q1);
            bezierControlPoints.push_back(q2);
            bezierControlPoints.push_back(q3);
        } else {
            bezierControlPoints.push_back(q1);
            bezierControlPoints.push_back(q2);
            bezierControlPoints.push_back(q3);
        }
    }

    // 4. Call evalBezier ONCE with the full list
    // This ensures the Normal/Binormal frames are consistent across the whole curve.
    return evalBezier(bezierControlPoints, steps);
}

Curve evalCircle( float radius, unsigned steps )
{
    // This is a sample function on how to properly initialize a Curve
    // (which is a vector< CurvePoint >).
    
    // Preallocate a curve with steps+1 CurvePoints
    Curve R( steps+1 );

    // Fill it in counterclockwise
    for( unsigned i = 0; i <= steps; ++i )
    {
        // step from 0 to 2pi
        float t = 2.0f * M_PI * float( i ) / steps;

        // Initialize position
        // We're pivoting counterclockwise around the y-axis
        R[i].V = radius * Vector3f( cos(t), sin(t), 0 );
        
        // Tangent vector is first derivative
        R[i].T = Vector3f( -sin(t), cos(t), 0 );
        
        // Normal vector is second derivative
        R[i].N = Vector3f( -cos(t), -sin(t), 0 );

        // Finally, binormal is facing up.
        R[i].B = Vector3f( 0, 0, 1 );
    }

    return R;
}

void drawCurve( const Curve& curve, float framesize )
{
    // Save current state of OpenGL
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    // Setup for line drawing
    glDisable( GL_LIGHTING ); 
    glColor4f( 1, 1, 1, 1 );
    glLineWidth( 1 );
    
    // Draw curve
    glBegin( GL_LINE_STRIP );
    for( unsigned i = 0; i < curve.size(); ++i )
    {
        glVertex( curve[ i ].V );
    }
    glEnd();

    glLineWidth( 1 );

    // Draw coordinate frames if framesize nonzero
    if( framesize != 0.0f )
    {
        Matrix4f M;

        for( unsigned i = 0; i < curve.size(); ++i )
        {
            M.setCol( 0, Vector4f( curve[i].N, 0 ) );
            M.setCol( 1, Vector4f( curve[i].B, 0 ) );
            M.setCol( 2, Vector4f( curve[i].T, 0 ) );
            M.setCol( 3, Vector4f( curve[i].V, 1 ) );

            glPushMatrix();
            glMultMatrixf( M );
            glScaled( framesize, framesize, framesize );
            glBegin( GL_LINES );
            glColor3f( 1, 0, 0 ); glVertex3d( 0, 0, 0 ); glVertex3d( 1, 0, 0 );
            glColor3f( 0, 1, 0 ); glVertex3d( 0, 0, 0 ); glVertex3d( 0, 1, 0 );
            glColor3f( 0, 0, 1 ); glVertex3d( 0, 0, 0 ); glVertex3d( 0, 0, 1 );
            glEnd();
            glPopMatrix();
        }
    }
    
    // Pop state
    glPopAttrib();
}

