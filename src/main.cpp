#include "Globals.hpp"
#include "View.hpp"
#include "World.hpp"
#include "Frame.hpp"
#include "Lights.hpp"
#include "core/Scene.hpp"

using namespace std;

//****************************************************
// Global Variables
//****************************************************

Scene * scene = NULL;
World * world = NULL;
View * view = NULL;
Mat4 viewToWorld = identity3D();
Frame * frame = NULL;
int max_trace_depth = 2;

// Get the shaded appearance of the primitive at a given position, as seen along a ray. The returned value should be the sum of
// the shaded colors w.r.t. each light in the scene. DO NOT include the result of recursive raytracing in this function, just
// use the ambient-diffuse-specular formula. DO include testing for shadows, individually for each light.
RGB
getShadedColor(Primitive const & primitive, Vec3 const & pos, Ray const & ray)
{
  Material objectMaterial = primitive.getMaterial();
  	RGB objectColor = primitive.getColor();
  	RGB materialS = objectMaterial.getMSM()*objectColor + (1 - objectMaterial.getMSM())*RGB(1,1,1);
  	RGB totalColorObject = objectMaterial.getMA()*objectColor*(*world).getAmbientLightColor();

  	Vec3 normal = primitive.calculateNormal(pos);
  	Vec3 viewingDir = ray.direction(); viewingDir.normalize();

	for(World::LightConstIterator i = world->lightsBegin(); i != world->lightsEnd(); ++i){

		bool isPointSource;
		int seed = rand()%100000;
		(*(*i)).setSeed(seed);
		std::vector<Ray> shadow = (*(*i)).getShadowRay(pos+0.0001*normal,isPointSource);
		std::vector<Vec3> lightDir = (*(*i)).getIncidenceVector(pos);
		srand(time(NULL));

		for(unsigned int j=0; j<shadow.size(); j++){
			Primitive* shadowObject = (*world).intersect(shadow[j]);
      // if shadow ray not intersects with anything
			if(shadowObject == NULL){
        // For area lights, we cannot average the phong colours from every sampled point
        // since more light is coming at the point; neither can we add all the light since
        // the image becomes too bright; we try to achieve a tradeoff by
        // introducing this factor

				RGB lightColor = (*(*i)).getColor(lightDir[j]); lightDir[j].normalize();
				RGB lambertianColorObject = objectMaterial.getML()*objectColor*lightColor*std::max(normal*lightDir[j],0.0);
				totalColorObject += lambertianColorObject/std::pow(shadow.size(),0.90);

				Vec3 reflectDir = -lightDir[j] + 2*(lightDir[j]*normal)*normal;
				reflectDir.normalize();
				RGB specularColorObject = objectMaterial.getMS()*materialS*lightColor*std::pow(std::max(-reflectDir*viewingDir,0.0),objectMaterial.getMSP());
				totalColorObject += specularColorObject/std::pow(shadow.size(),0.90);
			}
		}
	}

  return totalColorObject;
}

// Raytrace a single ray backwards into the scene, calculating the total color (summed up over all reflections/refractions) seen
// along this ray.
RGB
traceRay(Ray & ray, int depth)
{
  // Assumptions:
  // Refractive index of the space between objects is 1; we will call it air
  // Rays reflect and refract on hitting an object when in air
  // Rays will only refract on hitting air when travelling inside object
  // This is because we assume reflectivity of air to be 0
  // There is some air present between any two objects

  if (depth > max_trace_depth)
    return RGB(0, 0, 0);

  Primitive* object = (*world).intersect(ray);

  if(object != NULL){
  	Material objectMaterial = (*object).getMaterial();
  	RGB objectColor = (*object).getColor();
  	Vec3 primitiveHitPosition = ray.start() + ray.direction()*ray.minT();
		RGB totalColor = getShadedColor(*object, primitiveHitPosition, ray);
		RGB reflectedColor = RGB(0,0,0);
		RGB refractedColor = RGB(0,0,0);

		Vec3 primitiveHitNormal = (*object).calculateNormal(primitiveHitPosition);
    // if ray is present inside an object that i.e. it is refracted, the normal will be negative
    if(ray.isRefracted()){ primitiveHitNormal = -primitiveHitNormal; }
  	Vec3 viewingDir = ray.direction(); viewingDir.normalize();

		/** if ray present in air */
		if(!ray.isRefracted()){
      // generate a reflected ray
			Vec3 bounceDir = viewingDir - 2*(viewingDir*primitiveHitNormal)*primitiveHitNormal;
  		Vec3 bouncePos = primitiveHitPosition + 0.0001*primitiveHitNormal;
  		Ray bounceRay = Ray::fromOriginAndDirection(bouncePos,bounceDir);
			bounceRay.setRefracted(ray.isRefracted()); bounceRay.setEta(ray.getEta());
			reflectedColor = objectMaterial.getMR()*objectColor*traceRay(bounceRay,depth+1);
		}

		// generate refracted ray
		double cosTheta1 = -viewingDir*primitiveHitNormal;
		double eta1 = ray.getEta(), eta2;
		if(ray.isRefracted()){ eta2 = 1; } // if ray inside object, it must refract into air
		else{
			eta2 = objectMaterial.getMTN();
		}
		double sinTheta2sq = (eta1/eta2)*(eta1/eta2)*(1 - cosTheta1*cosTheta1);
		if(sinTheta2sq < 1){
			double cosTheta2 = std::sqrt(1 - sinTheta2sq);
			Vec3 refrDir = (eta1/eta2)*viewingDir + ((eta1/eta2)*cosTheta1 - cosTheta2)*primitiveHitNormal;
  		Vec3 refrPos = primitiveHitPosition - 0.0001*primitiveHitNormal;
  		Ray refrRay = Ray::fromOriginAndDirection(refrPos,refrDir);
			refrRay.setRefracted(!ray.isRefracted()); refrRay.setEta(eta2);
			refractedColor = objectMaterial.getMT()*objectColor*traceRay(refrRay,depth+1);
		}

  	return totalColor + reflectedColor + refractedColor;
  }

  return RGB(0,0,0);

  // Use the "world" global variable to access the primitives in the input file.
  //  IMPORTANT:
  //  Please start all bounce rays at a small non-zero t value such as 0.001 - this has the effect of slightly offsetting
  //  bounce rays from the surface they're bouncing from, and prevents bounce rays from being occluded by their own surface.
}

// Main rendering loop.
void
renderWithRaytracing()
{
  Sample sample;   // Point on the view being sampled.
  Ray ray;         // Ray being traced from the eye through the point.
  RGB c;           // Color being accumulated per pixel.

  int const rpp = view->raysPerPixel();

  for (int yi = 0; yi < view->height(); ++yi)
  {
    for (int xi = 0; xi < view->width(); ++xi)
    {
      c = RGB(0, 0, 0);
      for (int ri = 0; ri < rpp; ++ri)
      {
        view->getSample(xi, yi, ri, sample);
        ray = view->createViewingRay(sample);  // convert the 2d sample position to a 3d ray
        ray.transform(viewToWorld);            // transform this to world space
        c += traceRay(ray, 0);
      }

      frame->setColor(sample, c / (double)rpp);
			// std::cout << xi << " " << yi << std::endl;
  	}
  }
}

// This traverses the loaded scene file and builds a list of primitives, lights and the view object. See World.hpp.
void
importSceneToWorld(SceneInstance * inst, Mat4 localToWorld, int time)
{
  if (inst == NULL)
    return;

  Mat4 nodeXform;
  inst->computeTransform(nodeXform, time);
  localToWorld = localToWorld * nodeXform;
  SceneGroup * g = inst->getChild();

  if (g == NULL)   // for example if the whole scene fails to load
  {
    std::cout << "ERROR: We arrived at an instance with no child?!" << std::endl;
    return;
  }

  int ccount = g->getChildCount();

  for (int i = 0; i < ccount; i++)
  {
    importSceneToWorld(g->getChild(i), localToWorld, 0);
  }

  CameraInfo f;

  if (g->computeCamera(f, time))
  {
    viewToWorld = localToWorld;

    if (view != NULL)
      delete view;

    Vec3 eye(0.0, 0.0, 0.0);
    Vec3 LL(f.sides[FRUS_LEFT], f.sides[FRUS_BOTTOM], -f.sides[FRUS_NEAR]);
    Vec3 UL(f.sides[FRUS_LEFT], f.sides[FRUS_TOP], -f.sides[FRUS_NEAR]);
    Vec3 LR(f.sides[FRUS_RIGHT], f.sides[FRUS_BOTTOM], -f.sides[FRUS_NEAR]);
    Vec3 UR(f.sides[FRUS_RIGHT], f.sides[FRUS_TOP], -f.sides[FRUS_NEAR]);
    view = new View(eye, LL, UL, LR, UR, IMAGE_WIDTH, IMAGE_HEIGHT, RAYS_PER_PIXEL_EDGE);
  }

  LightInfo l;

  if (g->computeLight(l, time))
  {
    if (l.type == LIGHT_AMBIENT)
    {
      RGB amb = world->getAmbientLightColor();
      world->setAmbientLightColor(amb + l.color);
    }
    else if (l.type == LIGHT_DIRECTIONAL)
    {
      DirectionalLight * li = new DirectionalLight(l.color);
      Vec3 dir(0, 0, -1);
      li->setDirection(localToWorld * dir);
      world->addLight(li);
    }
    else if (l.type == LIGHT_POINT)
    {
      PointLight * li = new PointLight(l.color, l.falloff, l.deadDistance);
      Vec3 pos(0, 0, 0);
      li->setPosition(localToWorld * pos);
      world->addLight(li);
    }
		else if (l.type == LIGHT_AREA_SQUARE)
		{
			AreaLightSquare * li = new AreaLightSquare(l.color, l.falloff, l.deadDistance);
			Vec3 pos(0, 0, 0);
      li->setPosition(localToWorld * pos);
			// std::cout << l.side << std::endl;
			li->setSide(l.side);
      world->addLight(li);
		}
    else if (l.type == LIGHT_SPOT)
    {
      throw "oh no";
    }
  }

  double r;
  MaterialInfo m;

  if (g->computeSphere(r, m, time))
  {
    Material mat(m.k[0], m.k[1], m.k[2], m.k[3], m.k[4], m.k[MAT_MS], m.k[5], m.k[6]);
    Sphere * sph = new Sphere(r, m.color, mat, localToWorld);
    world->addPrimitive(sph);
  }

  TriangleMesh * t;

  if (g->computeMesh(t, m, time))
  {
    Material mat(m.k[0], m.k[1], m.k[2], m.k[3], m.k[4], m.k[MAT_MS], m.k[5], m.k[6]);

    for (vector<MeshTriangle *>::iterator it = t->triangles.begin(); it != t->triangles.end(); ++it)
    {
      Triangle * tri = new Triangle(
        t->vertices[ (**it).ind[0] ]->pos,
        t->vertices[ (**it).ind[1] ]->pos,
        t->vertices[ (**it).ind[2] ]->pos,
        m.color, mat, localToWorld);
      world->addPrimitive(tri);
			world->trianglesUpdate(tri);
    }
		world->trianglesComputeVertexNormal();
  }

  std::cout << "Imported scene file" << std::endl;
}

int
main(int argc, char ** argv)
{
  if (argc < 3)
  {
    std::cout << "Usage: " << argv[0] << " scene.scd output.png [max_trace_depth]" << std::endl;
    return -1;
  }

  if (argc >= 4)
    max_trace_depth = atoi(argv[3]);

  cout << "Max trace depth = " << max_trace_depth << endl;

  // Load the scene from the disk file
  scene = new Scene(argv[1]);

  // Setup the world object, containing the data from the scene
  world = new World();
  importSceneToWorld(scene->getRoot(), identity3D(), 0);
  world->printStats();

  // Set up the output framebuffer
  frame = new Frame(IMAGE_WIDTH, IMAGE_HEIGHT);

  // Render the world
  renderWithRaytracing();

  // Save the output to an image file
  frame->save(argv[2]);
  std::cout << "Image saved!" << std::endl;
}
