#pragma once
#include "globalThings.h"
#include "SmartArray.h"

class cPlant
{
public:
    const float GROWTH_LIMIT = 5.f;
    float limit_offset = 0.f;
    float growthTimer = 0.f;
    bool isRipe = false;
    cMesh* plantMesh = nullptr;

    cPlant()
    {
        limit_offset = (rand() % 3 - 1);
    }

    cPlant& operator= (cPlant& plant)
    {
        this->growthTimer = plant.growthTimer;
        this->plantMesh = plant.plantMesh;
        return *this;
    }

    void Update(float delta)
    {
        
        if (isRipe || growthTimer >= (GROWTH_LIMIT + limit_offset))
        {
            isRipe = true;
            this->plantMesh->textureNames[0] = "purple.bmp";
        }
        else
        {
            growthTimer += delta;
            this->plantMesh->scale.y += (delta / 6.0 * 5.0f / (GROWTH_LIMIT + limit_offset));
            this->plantMesh->positionXYZ.y += (delta / 6.0 * 5.0f / (GROWTH_LIMIT + limit_offset));
        }
    }

    void Consume()
    {
        growthTimer = 0;
        this->plantMesh->scale.y = 0.3f;
        this->isRipe = false;
        this->plantMesh->positionXYZ.y = 0.f;
        this->plantMesh->textureNames[0] = "Green_square.bmp";
    }
};

struct sPlantThreadDesc
{
    cSmartArray<cPlant>* plants;
    float constDeltaTima;
    HANDLE hWorkerThread;
    DWORD workerThreadID;
    // Pointer to the common bool for all the threads
    bool* pRunThreads;
    bool* pKillThreads;
    // Set to false right before the thread exits
    bool bIsAlive;

};

DWORD WINAPI PlantUpdateFunction(LPVOID lpParameter)
{
    sPlantThreadDesc* pPlantManagerThreadInfo = (sPlantThreadDesc*)lpParameter;
    Sleep(2000);
    while (!*(pPlantManagerThreadInfo->pKillThreads))
    {
        if (*(pPlantManagerThreadInfo->pRunThreads))
        {
            double delta = pPlantManagerThreadInfo->constDeltaTima;
            double sleepTime = 1000.0 * delta;

            for (int i = 0; i < pPlantManagerThreadInfo->plants->getSize(); i++)
            {
                pPlantManagerThreadInfo->plants->getAtIndex(i)->Update(delta);
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

class cPlantManager
{
public:
    cSmartArray<cPlant>* plants;
    int amountOfPlants = 10;
    sPlantThreadDesc* plantGrowthDesc;
    bool killThreads = false;
    bool runThreads = false;
    cPlantManager()
    {
        plants = new cSmartArray<cPlant>();
        plantGrowthDesc = new sPlantThreadDesc();
        plantGrowthDesc->plants = this->plants;
        plantGrowthDesc->constDeltaTima = 4.0 / 1000.0;
        plantGrowthDesc->pKillThreads = &(this->killThreads);
        plantGrowthDesc->pRunThreads = &(this->runThreads);

        plantGrowthDesc->hWorkerThread = CreateThread(
            NULL,
            0,
            PlantUpdateFunction,
            plantGrowthDesc,
            0,
            &(plantGrowthDesc->workerThreadID));
        
    }

    ~cPlantManager()
    {
        *(plantGrowthDesc->pKillThreads) = true;
        delete plants;
        delete plantGrowthDesc;
    }

    void createPlant(float coordX, float coordY)
    {
        cPlant plant;
        cMesh* plantM = new cMesh();
        plantM->meshName = "Isosphere_Smooth_Normals.ply";
        plantM->textureNames[0] = "Green_square.bmp";
        plantM->textureRatios[0] = 1.0f;
        plantM->scale = glm::vec3(0.3f);
        plantM->bDontLight = true;
        plantM->positionXYZ = glm::vec3(coordX, 0, coordY);
        ::g_vec_pMeshes.push_back(plantM);
        plant.plantMesh = plantM;
        plants->copyToEnd(plant);
    }


};