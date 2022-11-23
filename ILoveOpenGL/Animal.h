#pragma once
#include "globalThings.h"
#include "SmartArray.h"
#include "cPlant.h"

class Animal
{
	
public:
	float mHungerTimer = 0.f;
	float mMaxHunger = 0.f;
	bool mIsHungry = false;
	cMesh* animalMesh = nullptr;
	bool mHasSlept = false;
	bool mHasEaten = false;
	bool isAlive = true;
	bool isAfraid = false;
	glm::vec3 fearSource = glm::vec3(0);
	glm::vec3 destination = glm::vec3(0, -1, 0);
	Animal()
	{
	}

	void Update(float delta)
	{
		if (mIsHungry || mHungerTimer > mMaxHunger)
		{
 			mIsHungry = true;
		}
		else
		{
			mHungerTimer += delta;
		}
	}
};



struct sAnimalThreadDesc
{
	Animal* animal;
	float constDeltaTima = 4.f;
	HANDLE hWorkerThread;
	DWORD workerThreadID;
	// Pointer to the common bool for all the threads
	bool* pRunThreads;
	bool* pKillThreads;
	// Set to false right before the thread exits
	bool bIsAlive;
	void* targets;
	cSmartArray<cSmartArray<bool>>* maze;
	void* preyList;
};

static glm::vec3 pickDirectionAtDistance(float distance)
{
	int randomAngle = rand();
	glm::vec3 randomDirection = glm::vec3(glm::cos(randomAngle), 0.f, glm::sin(randomAngle));
	randomDirection = glm::normalize(randomDirection);
	randomDirection *= distance;
	return randomDirection;

}

static bool isGrass(float coordX, float coordY, cSmartArray<cSmartArray<bool>>* maze)
{
	int roundedX = (int)(coordX + 25.5f);
	int roundedY = (int)(coordY + 0.5f);
	if (*(maze->getAtIndex(roundedY)->getAtIndex(roundedX)))
	{
		return true;
	}
	else
	{
		return false;
	}
}

DWORD WINAPI PredatorUpdateFunction(LPVOID lpParameter)
{
	sAnimalThreadDesc* pPredatorInfo = (sAnimalThreadDesc*)lpParameter;
	Sleep(2000);
	cSmartArray<Animal>* prey = (cSmartArray<Animal>*)pPredatorInfo->targets;
	bool validTarget = false;
	while (!*(pPredatorInfo->pKillThreads))
	{
		if (*(pPredatorInfo->pRunThreads))
		{
			if (isGrass(pPredatorInfo->animal->animalMesh->positionXYZ.x, pPredatorInfo->animal->animalMesh->positionXYZ.z, pPredatorInfo->maze))
			{
				int randomNumber = rand() % 1000;
				if (randomNumber > 995)
				{
					pPredatorInfo->animal->animalMesh->textureRatios[0] = 0.3f;
					pPredatorInfo->animal->mHasSlept = true;
					Sleep(2000);
					pPredatorInfo->animal->animalMesh->textureRatios[0] = 1.0f;
					int predatorBirthChanceNumber = (rand() % 100);
					if (predatorBirthChanceNumber >= 98)
					{

						sAnimalThreadDesc* predatorDescThread = new sAnimalThreadDesc();
						Animal* predator = new Animal();
						cMesh* animesh = new cMesh();
						animesh->meshName = "dieS.ply";
						animesh->textureNames[0] = "blood.bmp";
						animesh->textureRatios[0] = 1.f;
						animesh->positionXYZ = pPredatorInfo->animal->animalMesh->positionXYZ;
						::g_vec_pMeshes.push_back(animesh);
						predator->animalMesh = animesh;
						predator->mMaxHunger = 12.f;
						predator->mIsHungry = false;

						predatorDescThread->animal = predator;
						predatorDescThread->bIsAlive = true;
						predatorDescThread->constDeltaTima = 4.0 / 1000.0;
						predatorDescThread->pKillThreads = pPredatorInfo->pKillThreads;
						predatorDescThread->pRunThreads = pPredatorInfo->pRunThreads;
						predatorDescThread->targets = pPredatorInfo->targets;
						predatorDescThread->maze = pPredatorInfo->maze;
						predatorDescThread->hWorkerThread = CreateThread(
							NULL,
							0,
							PredatorUpdateFunction,
							predatorDescThread,
							0,
							&(predatorDescThread->workerThreadID));
					}
					continue;
				}
			}
			double delta = pPredatorInfo->constDeltaTima;
			double sleepTime = 1000.0 * delta;
			int index = -1;
			pPredatorInfo->animal->Update(delta);
			if (pPredatorInfo->animal->mIsHungry)
			{
				//look for closest animal
				float minDistance = 50000.f;
				for (int i = 0; i < prey->getSize(); i++)
				{
					if (!(prey->getAtIndex(i)->isAlive))
						continue;
					float curDistance = glm::distance(pPredatorInfo->animal->animalMesh->positionXYZ, prey->getAtIndex(i)->animalMesh->positionXYZ);
					if (curDistance < minDistance)
					{
						minDistance = curDistance;
						index = i;
					}
				}
				if (index == -1)
				{
					//wander since prey is gone
					while (!validTarget)
					{
						pPredatorInfo->animal->destination = pPredatorInfo->animal->animalMesh->positionXYZ + pickDirectionAtDistance(1.f);
						if (pPredatorInfo->animal->destination.x > 25.f || pPredatorInfo->animal->destination.x < -25.f
							|| pPredatorInfo->animal->destination.z > 50.f || pPredatorInfo->animal->destination.z < 0.f)
							continue;
						validTarget = true;
					}
				}
				else
				{
					//get to the victim
					pPredatorInfo->animal->destination = prey->getAtIndex(index)->animalMesh->positionXYZ;
				}
			}
			else
			{
				//wander
				while (!validTarget)
				{
					pPredatorInfo->animal->destination = pPredatorInfo->animal->animalMesh->positionXYZ + pickDirectionAtDistance(1.f);
					if (pPredatorInfo->animal->destination.x > 25.f || pPredatorInfo->animal->destination.x < -25.f
						|| pPredatorInfo->animal->destination.z > 50.f || pPredatorInfo->animal->destination.z < 0.f)
						continue;
					validTarget = true;
				}
			}

			//move animal to it's target
			glm::vec3 direction = pPredatorInfo->animal->destination - pPredatorInfo->animal->animalMesh->positionXYZ;
			direction = glm::normalize(direction);
			direction *= delta;
			pPredatorInfo->animal->animalMesh->positionXYZ += direction;
			if (index >= 0)
			{
				if (glm::distance(pPredatorInfo->animal->animalMesh->positionXYZ, prey->getAtIndex(index)->animalMesh->positionXYZ) < 0.1f)
				{
					if (glm::distance(pPredatorInfo->animal->animalMesh->positionXYZ, prey->getAtIndex(index)->animalMesh->positionXYZ) < 0.6f)
					{
						prey->getAtIndex(index)->isAfraid = true;
						prey->getAtIndex(index)->fearSource = pPredatorInfo->animal->animalMesh->positionXYZ;
					}
					//horrible things happen
					prey->getAtIndex(index)->isAlive = false;

					pPredatorInfo->animal->mIsHungry = false;
					pPredatorInfo->animal->mMaxHunger = 30.f + (rand() % 20);
					pPredatorInfo->animal->mHungerTimer = 0.f;
					pPredatorInfo->animal->mHasEaten = true;
					validTarget = false;
				}
			}
			else
			{
				if (glm::distance(pPredatorInfo->animal->animalMesh->positionXYZ, pPredatorInfo->animal->destination) < 0.1f)
				{
					validTarget = false;//mark it as reached wander target

				}
				else while (!validTarget)
				{
					pPredatorInfo->animal->destination = pPredatorInfo->animal->animalMesh->positionXYZ + pickDirectionAtDistance(1.f);
					if (pPredatorInfo->animal->destination.x > 25.f || pPredatorInfo->animal->destination.x < -25.f
						|| pPredatorInfo->animal->destination.z > 50.f || pPredatorInfo->animal->destination.z < 0.f)
						continue;
					validTarget = true;
				}
			}


			Sleep(4);
		}
		else
		{
			Sleep(400);
		}
	}
	return 0;
}

DWORD WINAPI PreyUpdateFunction(LPVOID lpParameter)
{
	sAnimalThreadDesc* pPreyThreadDesc = (sAnimalThreadDesc*)lpParameter;
	Sleep(2000);
	cSmartArray<cPlant>* plants = (cSmartArray<cPlant>*)pPreyThreadDesc->targets;
	bool validTarget = false;
	float poopTimer = 0.f;
	while (!*(pPreyThreadDesc->pKillThreads))
	{
		if (*(pPreyThreadDesc->pRunThreads))
		{
			if (!(pPreyThreadDesc->animal->isAlive))
			{
				pPreyThreadDesc->animal->animalMesh->scale = glm::vec3(0.00001f);
				break;
			}
			double delta = pPreyThreadDesc->constDeltaTima;
			double sleepTime = 1000.0 * delta;
			int index = -1;
			pPreyThreadDesc->animal->Update(delta);
			if (pPreyThreadDesc->animal->mHasEaten)
			{
				poopTimer += delta;
				if (poopTimer > 40.f)
				{
					int poopRand = rand();
					if (poopTimer < 25)
					{
						//seed is planted
						cPlant plant;
						cMesh* plantM = new cMesh();
						plantM->meshName = "Isosphere_Smooth_Normals.ply";
						plantM->textureNames[0] = "Green_square.bmp";
						plantM->textureRatios[0] = 1.0f;
						plantM->scale = glm::vec3(0.3f);
						plantM->bDontLight = true;
						plantM->positionXYZ = pPreyThreadDesc->animal->animalMesh->positionXYZ;
						::g_vec_pMeshes.push_back(plantM);
						plant.plantMesh = plantM;
						plants->copyToEnd(plant);						
					}
					if (poopRand >= 90)
					{
						//seed has been planted
						sAnimalThreadDesc* preyDescThread = new sAnimalThreadDesc();

						Animal* prey = new Animal();
						cMesh* animesh = new cMesh();
						animesh->meshName = "dieS.ply";
						animesh->textureNames[0] = "Table_BaseColor.bmp";
						animesh->textureRatios[0] = 1.0f;
						animesh->positionXYZ = pPreyThreadDesc->animal->animalMesh->positionXYZ;
						animesh->scale = glm::vec3(1.f);
						::g_vec_pMeshes.push_back(animesh);
						prey->animalMesh = animesh;
						prey->mMaxHunger = 10.f;
						prey->mHungerTimer = 0.f;
						prey->mIsHungry = false;

						preyDescThread->animal = prey;
						preyDescThread->bIsAlive = true;
						preyDescThread->constDeltaTima = 4.0 / 1000.0;
						preyDescThread->pKillThreads = pPreyThreadDesc->pKillThreads;
						preyDescThread->pRunThreads = pPreyThreadDesc->pRunThreads;
						preyDescThread->targets = (void*)(pPreyThreadDesc->targets);
						preyDescThread->maze = pPreyThreadDesc->maze;
						preyDescThread->preyList = pPreyThreadDesc->preyList;
						preyDescThread->hWorkerThread = CreateThread(
							NULL,
							0,
							PreyUpdateFunction,
							preyDescThread,
							0,
							&(preyDescThread->workerThreadID));
						((cSmartArray<Animal>*)(preyDescThread->preyList))->insertAtEnd(prey);
					}
					poopTimer = 0.f;
				}
			}
			if (pPreyThreadDesc->animal->mIsHungry)
			{
				//look for plant that is ripe
				float minDistance = 50000.f;
				for (int i = 0; i < plants->getSize(); i++)
				{
					if (!(plants->getAtIndex(i)->isRipe))
					{
						continue;
					}
					float curDistance = glm::distance(pPreyThreadDesc->animal->animalMesh->positionXYZ, plants->getAtIndex(i)->plantMesh->positionXYZ);
					if (curDistance < minDistance)
					{
						minDistance = curDistance;
						index = i;
					}
				}
				if (index == -1)
				{
					//wander until something is ripe
					while ((!isGrass(pPreyThreadDesc->animal->destination.x, pPreyThreadDesc->animal->destination.z, pPreyThreadDesc->maze)) || !validTarget)
					{

						if (pPreyThreadDesc->animal->destination.x > 25.f || pPreyThreadDesc->animal->destination.x < -25.f
							|| pPreyThreadDesc->animal->destination.z > 50.f || pPreyThreadDesc->animal->destination.z < 0.f)
							continue;
						pPreyThreadDesc->animal->destination = pPreyThreadDesc->animal->animalMesh->positionXYZ + pickDirectionAtDistance(1.5f);
						validTarget = true;
					}
				}
				else
				{
					//get to the plant
					pPreyThreadDesc->animal->destination = plants->getAtIndex(index)->plantMesh->positionXYZ;
					pPreyThreadDesc->animal->destination.y = 0.f;
				}
			}
			else
			{
				//wander
				while ((!isGrass(pPreyThreadDesc->animal->destination.x, pPreyThreadDesc->animal->destination.z, pPreyThreadDesc->maze)) || !validTarget)
				{
					if (pPreyThreadDesc->animal->destination.x > 25.f || pPreyThreadDesc->animal->destination.x < -25.f
						|| pPreyThreadDesc->animal->destination.z > 50.f || pPreyThreadDesc->animal->destination.z < 0.f)
						continue;
					pPreyThreadDesc->animal->destination = pPreyThreadDesc->animal->animalMesh->positionXYZ + pickDirectionAtDistance(1.5f);
					validTarget = true;
				}
			}

			if (pPreyThreadDesc->animal->isAfraid)
			{
				//RUN
				glm::vec3 direction = pPreyThreadDesc->animal->animalMesh->positionXYZ - pPreyThreadDesc->animal->fearSource ;
				direction = glm::normalize(direction);
				direction *= delta;
				pPreyThreadDesc->animal->animalMesh->positionXYZ += direction;
			}
			else
			{
				//move animal to it's target
				glm::vec3 direction = pPreyThreadDesc->animal->destination - pPreyThreadDesc->animal->animalMesh->positionXYZ;
				direction = glm::normalize(direction);
				direction *= delta;
				pPreyThreadDesc->animal->animalMesh->positionXYZ += direction;				

			}
			if (index >= 0)
			{
				if (glm::distance(pPreyThreadDesc->animal->animalMesh->positionXYZ, plants->getAtIndex(index)->plantMesh->positionXYZ) < 1.1f)
				{
					plants->getAtIndex(index)->Consume();
					pPreyThreadDesc->animal->mIsHungry = false;
					pPreyThreadDesc->animal->mMaxHunger = 15.f + (rand() % 15);
					pPreyThreadDesc->animal->mHungerTimer = 0.f;
					pPreyThreadDesc->animal->mHasEaten = true;
					validTarget = false;

				}
			}
			else
			{
				if (glm::distance(pPreyThreadDesc->animal->animalMesh->positionXYZ, pPreyThreadDesc->animal->destination) < 0.1f)
				{
					validTarget = false;//mark it as reached wander target

				}
				else if (!isGrass(pPreyThreadDesc->animal->animalMesh->positionXYZ.x, pPreyThreadDesc->animal->animalMesh->positionXYZ.z, pPreyThreadDesc->maze))
				{
					while ((!isGrass(pPreyThreadDesc->animal->destination.x, pPreyThreadDesc->animal->destination.z, pPreyThreadDesc->maze)) || !validTarget)
					{
						if (pPreyThreadDesc->animal->destination.x > 25.f || pPreyThreadDesc->animal->destination.x < -25.f
							|| pPreyThreadDesc->animal->destination.z > 50.f || pPreyThreadDesc->animal->destination.z < 0.f)
							continue;
						pPreyThreadDesc->animal->destination = pPreyThreadDesc->animal->animalMesh->positionXYZ + pickDirectionAtDistance(1.f);
						validTarget = true;
					}
				}
			}

			Sleep(4);
		}
		else
		{
			Sleep(400);
		}
	}
	return 0;
};

class AnimalManager
{
public:
	cSmartArray<cSmartArray<bool>>* mazeInfo;
	cSmartArray<Animal> predatorsArr;
	cSmartArray<Animal> preyArr;
	cSmartArray<sAnimalThreadDesc> predatorThreads;
	cSmartArray<sAnimalThreadDesc> preyThreads;
	cPlantManager* plantInfo;
	bool killThreads = false;
	bool runThreads = false;

	AnimalManager(cPlantManager& plantManager, cSmartArray<cSmartArray<bool>>& maze)
	{
		plantInfo = &plantManager;
		mazeInfo = &maze;
	}

	~AnimalManager()
	{
		killThreads = true;
	}

	void makePredator(int coordX, int coordY)
	{
		sAnimalThreadDesc* predatorDescThread = new sAnimalThreadDesc();
		Animal* predator = new Animal();
		cMesh* animesh = new cMesh();
		animesh->meshName = "dieS.ply";
		animesh->textureNames[0] = "blood.bmp";
		animesh->textureRatios[0] = 1.f;
		animesh->positionXYZ = glm::vec3(coordX, 0.f, coordY);
		::g_vec_pMeshes.push_back(animesh);
		predator->animalMesh = animesh;
		predator->mMaxHunger = 11.f + (rand() % 8);
		predator->mIsHungry = false;

		predatorDescThread->animal = predator;
		predatorDescThread->bIsAlive = true;
		predatorDescThread->constDeltaTima = 4.0 / 1000.0;
		predatorDescThread->pKillThreads = &(killThreads);
		predatorDescThread->pRunThreads =  &(runThreads);
		predatorDescThread->targets =  &preyArr;
		predatorDescThread->maze = this->mazeInfo;
		predatorDescThread->hWorkerThread = CreateThread(
			NULL,
			0,
			PredatorUpdateFunction,
			predatorDescThread,
			0,
			&(predatorDescThread->workerThreadID));
		predatorThreads.insertAtEnd(predatorDescThread);
		predatorsArr.insertAtEnd(predatorDescThread->animal);
	}

	void makePrey(int coordX, int coordY)
	{
		sAnimalThreadDesc* preyDescThread = new sAnimalThreadDesc();

		Animal* prey = new Animal();
		cMesh* animesh = new cMesh();
		animesh->meshName = "dieS.ply";
		animesh->textureNames[0] = "Table_BaseColor.bmp";
		animesh->textureRatios[0] = 1.0f;
		animesh->positionXYZ = glm::vec3(coordX, 0.f, coordY);
		animesh->scale = glm::vec3(1.f);
		::g_vec_pMeshes.push_back(animesh);
		prey->animalMesh = animesh;
		prey->mMaxHunger = 5.f + (rand() % 5);
		prey->mHungerTimer = 0.f;
		prey->mIsHungry = false;

		preyDescThread->animal = prey;
		preyDescThread->bIsAlive = true;
		preyDescThread->constDeltaTima = 4.0 / 1000.0;
		preyDescThread->pKillThreads = &(killThreads);
		preyDescThread->pRunThreads = &(runThreads);
		preyDescThread->targets = (void*)(plantInfo->plants);
		preyDescThread->maze = this->mazeInfo;
		preyDescThread->hWorkerThread = CreateThread(
			NULL,
			0,
			PreyUpdateFunction,
			preyDescThread,
			0,
			&(preyDescThread->workerThreadID));
		preyThreads.insertAtEnd(preyDescThread);
		preyArr.insertAtEnd(preyDescThread->animal);
		preyDescThread->preyList = &preyArr;
	}
};
