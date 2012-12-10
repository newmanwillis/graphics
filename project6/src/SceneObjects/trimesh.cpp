#include <cmath>
#include <float.h>
#include "trimesh.h"

using namespace std;

Trimesh::~Trimesh()
{
	for( Materials::iterator i = materials.begin(); i != materials.end(); ++i )
		delete *i;
}

// must add vertices, normals, and materials IN ORDER
void Trimesh::addVertex( const Vec3d &v )
{
    vertices.push_back( v );
}

void Trimesh::addMaterial( Material *m )
{
    materials.push_back( m );
}

void Trimesh::addNormal( const Vec3d &n )
{
    normals.push_back( n );
}

// Returns false if the vertices a,b,c don't all exist
bool Trimesh::addFace( int a, int b, int c )
{
    int vcnt = vertices.size();

    if( a >= vcnt || b >= vcnt || c >= vcnt ) return false;

    TrimeshFace *newFace = new TrimeshFace( scene, new Material(*this->material), this, a, b, c );
    newFace->setTransform(this->transform);
    faces.push_back( newFace );
    return true;
}

char *
Trimesh::doubleCheck()
// Check to make sure that if we have per-vertex materials or normals
// they are the right number.
{
    if( !materials.empty() && materials.size() != vertices.size() )
        return "Bad Trimesh: Wrong number of materials.";
    if( !normals.empty() && normals.size() != vertices.size() )
        return "Bad Trimesh: Wrong number of normals.";

    return 0;
}

bool Trimesh::intersectLocal(const ray&r, isect&i) const
{
	double tmin = 0.0;
	double tmax = 0.0;
	typedef Faces::const_iterator iter;
	bool have_one = false;
	for( iter j = faces.begin(); j != faces.end(); ++j ) {
		isect cur;
		if( (*j)->intersectLocal( r, cur ) )
		{
			if( !have_one || (cur.t < i.t) )
			{
				i = cur;
				have_one = true;
			}
		}
	}
	if( !have_one ) i.setT(1000.0);
	return have_one;
}

// Intersect ray r with the triangle abc.  If it hits returns true,
// and puts the t parameter, barycentric coordinates, normal, object id,
// and object material in the isect object
bool TrimeshFace::intersectLocal( const ray& r, isect& i ) const
{
  const Vec3d& a = parent->vertices[ids[0]];
  const Vec3d& b = parent->vertices[ids[1]];
  const Vec3d& c = parent->vertices[ids[2]];
    
  Vec3d P = r.getPosition();
  float t = (dist-normal*P)/(normal*r.getDirection());
  Vec3d p = P + t*r.getDirection();
  
  if (t<=100*RAY_EPSILON)
    return false;

  // calculate barycentric coordinates
  Vec3d n = (b-a)^(c-a);
  Vec3d na = (c-b) ^ (p-b);
  Vec3d nb = (a-c) ^ (p-c);
  Vec3d nc = (b-a) ^ (p-a);
  double u = n*na/(n.length2());
  double v = n*nb/(n.length2());
  double w = n*nc/(n.length2());
  Vec3d barycentric_coordinates = Vec3d(u, v, w);
  // The barycentric coordinates were calculated using the equations from 
  // http://facultyfp.salisbury.edu/despickler/personal/Resources/Graphics/Resources/barycentric.pdf

  // Check if barycentric coordinates are within [0,1]
  if (barycentric_coordinates[0]<0.0 || barycentric_coordinates[0]>1.0)
     return false;
  if (barycentric_coordinates[1]<0.0|| barycentric_coordinates[1]>1.0)
    return false;
  if (barycentric_coordinates[2]<0.0|| barycentric_coordinates[2]>1.0)
    return false;

  // interpolate across normals
  Vec3d final_norm = normal;
  if (!parent->normals.empty()) {
    final_norm = u*parent->normals[ids[0]] + 
      v*parent->normals[ids[1]] + w*parent->normals[ids[2]];
  }

  i.setBary(barycentric_coordinates);
  i.setN(final_norm);
  i.setT(t);
  i.setObject(this);

  // interpolate across materials
  Material mate = Material();
  if (!parent->materials.empty()) {
    mate += u**(parent->materials[ids[0]]);
    mate += v**(parent->materials[ids[1]]);
    mate += w**(parent->materials[ids[2]]);
    i.setMaterial(mate);
  } 
  return true;
}

void Trimesh::generateNormals()
// Once you've loaded all the verts and faces, we can generate per
// vertex normals by averaging the normals of the neighboring faces.
{
    int cnt = vertices.size();
    normals.resize( cnt );
    int *numFaces = new int[ cnt ]; // the number of faces assoc. with each vertex
    memset( numFaces, 0, sizeof(int)*cnt );
    
    for( Faces::iterator fi = faces.begin(); fi != faces.end(); ++fi )
    {
		Vec3d faceNormal = (**fi).getNormal();
        
        for( int i = 0; i < 3; ++i )
        {
            normals[(**fi)[i]] += faceNormal;
            ++numFaces[(**fi)[i]];
        }
    }

    for( int i = 0; i < cnt; ++i )
    {
        if( numFaces[i] )
            normals[i]  /= numFaces[i];
    }

    delete [] numFaces;
    vertNorms = true;
}

