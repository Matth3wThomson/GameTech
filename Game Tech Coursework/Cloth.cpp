#include "Cloth.h"


Cloth::Cloth(int width, int height, const Vector3& pos, const Vector3& scale, float sphereRadius, const std::string& filename)
{
	//Calculate the number of nodes we will need to store
	nodes.reserve(width*height);

	gridMesh = new GridMesh(width, height);

	gridMesh->SetTexture(SOIL_load_OGL_texture(filename.c_str(), 
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	Matrix4 vertPos = Matrix4::Translation(pos)
		* Matrix4::Rotation(90, Vector3(0,0,1))
		* Matrix4::Scale(scale);

	for (unsigned int i=0; i<gridMesh->numVertices; ++i){
		nodes.push_back(new PhysicsNode());
		nodes.back()->SetPosition(vertPos * gridMesh->vertices[i]);
		nodes.back()->SetBroadPhaseVolume(
			new CollisionSphere(nodes.back()->GetPosition(), sphereRadius));


		nodes.back()->SetScale(Vector3(sphereRadius, sphereRadius, sphereRadius));
		nodes.back()->SetMass(1.0f);
		nodes.back()->SetInvSphereInertiaMatrix(1.0f, sphereRadius);
		nodes.back()->SetTarget(NULL);

		nodes.back()->SetConstantAccel(Vector3(0, -0.01f, 0));
		//nodes.back()->SetFixed(true);
	}

	for (auto itr = nodes.begin(); itr != nodes.end(); ++itr){
		std::cout << (*itr)->GetPosition() << std::endl;
	}

	for (int i=0; i<width; ++i){
		for (int j=0; j<height; ++j){
			int offset = (j * width) + i;
			PhysicsNode* pn = nodes[offset];

			//Can we link left?
			if (i != 0){
				Spring* s = new Spring(pn, Vector3(0,0,0),
					nodes[(width*j)+i-1], Vector3(0,0,0));
				springs.push_back(s);
			}

			if (i == width-1)
				pn->SetFixed(true);

			//Can we link down?
			if (j != 0){
				Spring* s = new Spring(pn, Vector3(0,0,0),
					nodes[(width*(j-1))+i], Vector3(0,0,0));
				springs.push_back(s);
			}
		}
	}

	cloth = new SceneNode(gridMesh);
	cloth->SetBoundingRadius(scale.Length() * 2.0f);
	cloth->SetShader(Renderer::GetRenderer().phong);
	///*cloth->SetShader(Renderer::GetRenderer().sceneNoBumpShader);
	//cloth->SetShaderUpdateFunc( []{ Renderer::GetRenderer().UpdateCombineSceneShaderMatricesPO(); });*/
}


Cloth::~Cloth(void)
{

}

void Cloth::Update(float msec){
	
	for (int i=0; i<gridMesh->numVertices; ++i){
		gridMesh->vertices[i] = nodes[i]->GetPosition();
	}
	gridMesh->RebufferData();
}

void Cloth::ConnectToSystems(){

	PhysicsSystem& ps = PhysicsSystem::GetPhysicsSystem();

	for (auto itr = nodes.begin(); itr != nodes.end(); ++itr){
		ps.AddNode(*itr);
	}

	for (auto itr = springs.begin(); itr != springs.end(); ++itr){
		ps.AddConstraint(*itr);
	}

	Renderer::GetRenderer().AddNode(cloth);
}

void Cloth::DisconnectFromSystems(){

	for (auto itr = springs.begin(); itr != springs.end(); ++itr){
		PhysicsSystem::GetPhysicsSystem().RemoveConstraint(*itr);
	}

	for (auto itr = nodes.begin(); itr != nodes.end(); ++itr){
		PhysicsSystem::GetPhysicsSystem().RemoveNode(*itr);
	}
}
