#include "rope.h"

unique_ptr<Entity> rope::CreateParticle(int x, int y, int z, bool horizontal, int fixedPoints)
{
	unique_ptr<Entity> ent(new Entity());
	ent->SetName("Particle");
	if (horizontal)
		ent->SetPosition(vec3(-10 * x, y, z));
	else
		ent->SetPosition(vec3(x, 10 * y, z));
	//ent->SetPosition(vec3(0, 2.0, 0));
	unique_ptr<Component> physComponent(new cPhysics());
	unique_ptr<cShapeRenderer> renderComponent(new cShapeRenderer(cShapeRenderer::SPHERE));
	renderComponent->SetColour(phys::RandomColour());

	ent->AddComponent(physComponent);

	auto b = ent->GetComponents("Physics");
	auto p = static_cast<cPhysics *>(b[0]);

	if (fixedPoints >= 2)
	{
		if (x == 0 || x == 19)
		{
			p->fake = true;
		}
		else
		{
			p->fake = false;
			p->mass = 1;
		}
	}
	else
	{
		if (y == 9)
		{
			p->fake = true;
			cout << "made a fake particle" << endl;
		}
		else
		{
			p->fake = false;
			p->mass = 1;
		}
	}

	ent->AddComponent(unique_ptr<Component>(new cSphereCollider()));
	auto collComp = ent->GetComponents("SphereCollider");
	auto coll = static_cast<cSphereCollider *>(collComp[0]); // Get Collider component
	coll->radius = 0.5;
	ent->AddComponent(unique_ptr<Component>(move(renderComponent)));

	return ent;
}

// Not used
void rope::updateBridgeSpringForces(vector<ParticleSpring> springs1, vector<ParticleSpring> springs2, vector<ParticleSpring> springs3,
	vector<unique_ptr<Entity>> particlePositions1, vector<unique_ptr<Entity>> particlePositions2, bool cut)
{
	for (int i = 0; i < springs1.size() - 1; i++)
	{
		if (cut && i == particlePositions1.size() / 2)
		{
			continue;
		}
		auto b = particlePositions1[i].get()->GetComponents("Physics");
		auto p = static_cast<cPhysics *>(b[0]);

		springs1[i + 1].updateForce(p, 1.0f);

		auto b2 = particlePositions2[i].get()->GetComponents("Physics");
		auto p2 = static_cast<cPhysics *>(b2[0]);

		springs2[i + 1].updateForce(p2, 1.0f);

		springs3[i + 1].updateForce(p2, 1.0f);

	}
}

unique_ptr<Entity> rope::CreatePlanks()
{
	unique_ptr<Entity> ent(new Entity());
	ent->SetName("Plank");
	ent->SetPosition(vec3(0, 0, 0));
	ent->SetScale(vec3(6.0f, 0.5f, 7.0f));
	unique_ptr<Component> physComponent(new cPlank());
	unique_ptr<cShapeRenderer> renderComponent(new cShapeRenderer(cShapeRenderer::BOX));

	renderComponent->SetColour(phys::RandomColour());

	ent->AddComponent(physComponent);

	ent->AddComponent(unique_ptr<Component>(new cAABB()));

	ent->AddComponent(unique_ptr<Component>(move(renderComponent)));

	return ent;

}