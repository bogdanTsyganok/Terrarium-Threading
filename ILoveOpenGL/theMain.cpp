
#include "GLCommon.h"
//#include <glad/glad.h>
//#define GLFW_INCLUDE_NONE
//#include <GLFW/glfw3.h>

#include "globalThings.h"

//#include "linmath.h"
#include <glm/glm.hpp>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> 
// glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr


#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>      // "string stream" just like iostream and fstream
#include <vector>       // "smart array"
#include <fstream>      // C++ file I-O library (look like any other stream)
#include "cAnimationControl.h"
#include "MazeLoader/cMazeMaker.h"
#include "SmartArray.h"
#include "Animal.h"

// Are in globalThings.h
//#include "cMesh.h"
//#include "cVAOManager.h"
//#include "cShaderManager.h"
//
//#include "cLightManager.h"
//#include "cLightHelper.h"

// Function signature for DrawObject()
void DrawObject(
    cMesh* pCurrentMesh, 
    glm::mat4 matModel,
    GLint matModel_Location,
    GLint matModelInverseTranspose_Location,
    GLuint program,
    cVAOManager* pVAOManager);


void DrawDebugObjects(
    GLint matModel_Location,
    GLint matModelInverseTranspose_Location,
    GLuint program,
    cVAOManager* pVAOManager);

//// Global things are here:
//glm::vec3 cameraEye = glm::vec3(0.0, 0.0, +4.0f);
//
//cVAOManager*    g_pVAOManager;
//cShaderManager*  g_pShaderManager;
//
//cLightManager gTheLights;
//cLightHelper gTheLightHelper;
//
//// List of objects to draw
//std::vector< cMesh* > g_vec_pMeshes;
////std::vector<cMesh> g_vecMeshes;
//
//cMesh* g_pDebugSphere = NULL;
//bool g_bShowDebugShere = true;
//
//
//unsigned int g_selectedObject = 0;
//unsigned int g_selectedLight = 0;
//
//
//// This will be printed in the title bar
//std::string g_TitleText = "";

float distanceToCamera(glm::vec3 pos)
{
    return glm::distance(::g_pFlyCamera->getEye(), pos);
}



void LoadTheAnimationCommands(cAnimationControl* pAnimationControl);

// This is used for the discard example on November 23rd
float g_blackHoleSize_for_discard_example = 0.0f;


cSmartArray<cSmartArray<bool>> smartMaze;



int main(void)
{
    cMazeMaker mazeMaker;
    mazeMaker.GenerateMaze(50, 50);

    for (int i = 0; i < mazeMaker.maze.size(); i++)
    {
        cSmartArray<bool>* mazeRow = new cSmartArray<bool>();
        for (int k = 0; k < mazeMaker.maze[i].size(); k++)
        {
            bool mazeSlot;
            mazeSlot = mazeMaker.maze[i][k][0];
            mazeRow->copyToEnd(mazeSlot);
        }
        smartMaze.insertAtEnd(mazeRow);
    }

    cPlantManager plantManager;
    for (int i = 0; i < plantManager.amountOfPlants; i++)
    {
        int coordX = 0, coordY = 0;
        bool plantPlaceFound = false;
        while (!plantPlaceFound)
        {
            coordX = rand() % 50;
            coordY = rand() % 50;
            if (*(smartMaze.getAtIndex(coordY)->getAtIndex(coordX)))
            {
                plantPlaceFound = true;
                plantManager.createPlant(coordX - 25, coordY);
            }
        }
    }

    AnimalManager animalManager(plantManager, smartMaze);
    

    for (int i = 0; i < 5; i++)
    {
        int coordX = 0, coordY = 0;
        bool plantPlaceFound = false;
        while (!plantPlaceFound)
        {
            coordX = rand() % 50;
            coordY = rand() % 50;
            if (*(smartMaze.getAtIndex(coordY)->getAtIndex(coordX)))
            {
                plantPlaceFound = true;
                animalManager.makePrey(coordX - 25, coordY);
            }
        }
    }

    for (int i = 0; i < 3; i++)
    {
        int coordX = 0, coordY = 0;
        bool plantPlaceFound = false;
        while (!plantPlaceFound)
        {
            coordX = rand() % 50;
            coordY = rand() % 50;
            if (!(*(smartMaze.getAtIndex(coordY)->getAtIndex(coordX))))
            {
                plantPlaceFound = true;
                animalManager.makePredator(coordX - 25, coordY);
            }
        }
    }

    GLFWwindow* pWindow;

//    GLuint vertex_buffer = 0;     // ** NOW in VAO Manager **

//    GLuint vertex_shader;     // Now in the "Shader Manager"
//    GLuint fragment_shader;   // Now in the "Shader Manager"
    GLuint program = 0;     // 0 means "no shader program"

    GLint mvp_location = -1;        // Because glGetAttribLocation() returns -1 on error
//    GLint vpos_location = -1;       // Because glGetAttribLocation() returns -1 on error
//    GLint vcol_location = -1;       // Because glGetAttribLocation() returns -1 on error

    glfwSetErrorCallback(GLFW_error_callback);

    if ( ! glfwInit() )
    {
        return -1;
        //exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    pWindow = glfwCreateWindow(1200, 640, "OpenGL is great!", NULL, NULL);

    if (!pWindow )
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(pWindow, GLFW_key_callback);
    // NEW: 
    glfwSetCursorEnterCallback( pWindow, GLFW_cursor_enter_callback );
    glfwSetCursorPosCallback( pWindow, GLFW_cursor_position_callback );
    glfwSetScrollCallback( pWindow, GLFW_scroll_callback );
    glfwSetMouseButtonCallback( pWindow, GLFW_mouse_button_callback );
    glfwSetWindowSizeCallback( pWindow, GLFW_window_size_callback );

    glfwMakeContextCurrent(pWindow);
    gladLoadGLLoader( (GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(1);


    // void glGetIntegerv(GLenum pname, GLint * data);
    // See how many active uniform variables (registers) I can have
    GLint max_uniform_location = 0;
    GLint* p_max_uniform_location = NULL;
    p_max_uniform_location = &max_uniform_location;
    glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, p_max_uniform_location);

    // You'll actually see something like this:
//    GLint max_uniform_location = 0;
//    glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, &max_uniform_location);

    std::cout << "GL_MAX_UNIFORM_LOCATIONS: " << max_uniform_location << std::endl;

    // Create global things
    ::g_StartUp(pWindow);

    ::g_pFlyCamera->setEye( glm::vec3(0.0f, 10.0f, -20.0f) );
//    ::g_pFlyCamera->setEye( glm::vec3(0.0f, 5000.0f, -3000.0f) );


    cShaderManager::cShader vertShader;
    vertShader.fileName = "assets/shaders/vertShader_01.glsl";
        
    cShaderManager::cShader fragShader;
    fragShader.fileName = "assets/shaders/fragShader_01.glsl";

    if (::g_pShaderManager->createProgramFromFile("Shader#1", vertShader, fragShader))
    {
        std::cout << "Shader compiled OK" << std::endl;
        // 
        // Set the "program" variable to the one the Shader Manager used...
        program = ::g_pShaderManager->getIDFromFriendlyName("Shader#1");
    }
    else
    {
        std::cout << "Error making shader program: " << std::endl;
        std::cout << ::g_pShaderManager->getLastError() << std::endl;
    }


    // Select the shader program we want to use
    // (Note we only have one shader program at this point)
    glUseProgram(program);


    // ************************************************
    // Now, I'm going to "load up" all the uniform locations
    // (This was to show how a map could be used)
    cShaderManager::cShaderProgram* pShaderProc = ::g_pShaderManager->pGetShaderProgramFromFriendlyName("Shader#1");

    int theUniformIDLoc = -1;
    theUniformIDLoc = glGetUniformLocation(program, "matModel");
    pShaderProc->mapUniformName_to_UniformLocation["matModel"] = theUniformIDLoc;
    
    // Or...
    pShaderProc->mapUniformName_to_UniformLocation["matModel"] = glGetUniformLocation(program, "matModel");
    pShaderProc->mapUniformName_to_UniformLocation["matView"] = glGetUniformLocation(program, "matView");
    pShaderProc->mapUniformName_to_UniformLocation["matProjection"] = glGetUniformLocation(program, "matProjection");
    pShaderProc->mapUniformName_to_UniformLocation["matModelInverseTranspose"] = glGetUniformLocation(program, "matModelInverseTranspose");

    pShaderProc->mapUniformName_to_UniformLocation["wholeObjectSpecularColour"] = glGetUniformLocation(program, "wholeObjectSpecularColour");
    // .. and so on...
    pShaderProc->mapUniformName_to_UniformLocation["theLights[0].position"] = glGetUniformLocation(program, "wholeObjectSpecularColour");
    // ... and so on..
    // ************************************************

    //GLint mvp_location = -1;
    mvp_location = glGetUniformLocation(program, "MVP");

    // Get "uniform locations" (aka the registers these are in)
    GLint matModel_Location = glGetUniformLocation(program, "matModel");
//    GLint matView_Location = glGetUniformLocation(program, "matView");
    GLint matProjection_Location = glGetUniformLocation(program, "matProjection");
    GLint matModelInverseTranspose_Location = glGetUniformLocation(program, "matModelInverseTranspose");


    cAnimationControl* animationController = new cAnimationControl();

    ::g_pTheLights->theLights[0].position = glm::vec4(100.0f, 100.0f, 0.0f, 1.0f);
    ::g_pTheLights->theLights[0].atten.y = 0.0001f;
    ::g_pTheLights->theLights[0].atten.z = 0.00001f;
    ::g_pTheLights->theLights[0].diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
//    
    ::g_pTheLights->theLights[0].param1.x = 2.0f;    // directional
    ::g_pTheLights->theLights[0].direction = glm::vec4(0.0f, -1.0f, 0.0f, 1.0f);
    ::g_pTheLights->theLights[0].diffuse = glm::vec4(0.9922f, 0.9843f, 0.9275f, 1.0f);
    ::g_pTheLights->theLights[0].param2.x = 1.f;
    // Get the uniform locations of the light shader values
    ::g_pTheLights->SetUpUniformLocations(program);

    
    // Set up the debug sphere object
    ::g_pDebugSphere = new cMesh();
    ::g_pDebugSphere->meshName = "Sphere_xyz_n_rgba.ply";
    ::g_pDebugSphere->bIsWireframe = true;
    ::g_pDebugSphere->bUseObjectDebugColour = true;
    ::g_pDebugSphere->objectDebugColourRGBA = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    ::g_pDebugSphere->bDontLight = true;
    ::g_pDebugSphere->scale = glm::vec3(1.0f);
    ::g_pDebugSphere->positionXYZ = ::g_pTheLights->theLights[0].position;
    // Give this a friendly name
    ::g_pDebugSphere->friendlyName = "Debug Sphere";
    // Then I could make a small function that searches for that name


    


    ::g_pVAOManager->setFilePath("assets/models/");

    std::vector<std::string> vecModelsToLoad;
    //vecModelsToLoad.push_back("Sphere_xyz_n_rgba.ply");
    vecModelsToLoad.push_back("ISO_Shphere_flat_3div_xyz_n_rgba_uv.ply");
    vecModelsToLoad.push_back("ISO_Shphere_flat_4div_xyz_n_rgba_uv.ply");
    vecModelsToLoad.push_back("Sphere_xyz_n_rgba.ply");
    vecModelsToLoad.push_back("Cylinder.ply");
    vecModelsToLoad.push_back("dieS.ply");
    vecModelsToLoad.push_back("Quad_1_sided_aligned_on_XY_plane.ply");

    vecModelsToLoad.push_back("Isosphere_Smooth_Normals.ply");
    vecModelsToLoad.push_back("Isosphere_Smooth_Inverted_Normals_for_SkyBox.ply");




    unsigned int totalVerticesLoaded = 0;
    unsigned int totalTrianglesLoaded = 0;
    for (std::vector<std::string>::iterator itModel = vecModelsToLoad.begin(); itModel != vecModelsToLoad.end(); itModel++)
    {
        sModelDrawInfo theModel;
        std::string modelName = *itModel;
        std::cout << "Loading " << modelName << "...";
        if ( !::g_pVAOManager->LoadModelIntoVAO(modelName, theModel, program) )
        {
            std::cout << "didn't work because: " << std::endl;
            std::cout << ::g_pVAOManager->getLastError(true) << std::endl;
        }
        else
        {
            std::cout << "OK." << std::endl;
            std::cout << "\t" << theModel.numberOfVertices << " vertices and " << theModel.numberOfTriangles << " triangles loaded." << std::endl;
            totalTrianglesLoaded += theModel.numberOfTriangles;
            totalVerticesLoaded += theModel.numberOfVertices;
        }

//        ::g_pVAOManager->LoadPLYModelFromFile(modelName, theModel);
//        ::g_pVAOManager->GenerateSphericalTextureCoords(theModel, false, 1.0f, false);
//        ::g_pVAOManager->LoadModelIntoVAO(theModel, program);

    }//for (std::vector<std::string>::iterator itModel


    std::cout << "Done loading models." << std::endl;
    std::cout << "Total vertices loaded = " << totalVerticesLoaded << std::endl;
    std::cout << "Total triangles loaded = " << totalTrianglesLoaded << std::endl;


   


    for (int i = 0; i < smartMaze.getSize(); i++)
    {
        for (int k = 0; k < smartMaze.getAtIndex(i)->getSize(); k++)
        {
            cMesh* pMazeSquare = new cMesh();
            pMazeSquare->meshName = "Quad_1_sided_aligned_on_XY_plane.ply";
            pMazeSquare->positionXYZ = glm::vec3(-25.f + k, 0.f, 0.f + i);
            pMazeSquare->orientationXYZ = glm::vec3(glm::radians(-90.f), 0.f, 0.f);
            if (*(smartMaze.getAtIndex(i)->getAtIndex(k)))
            {
                pMazeSquare->textureNames[0] = "Green_square.bmp";
            }
            else
            {
                pMazeSquare->textureNames[0] = "Yellow_square.bmp";
            }
            pMazeSquare->textureRatios[0] = 1.0f;
            pMazeSquare->bDontLight = true;
            //pMazeSquare->bDontLight = true;
            ::g_vec_pMeshes.push_back(pMazeSquare);
        }
    }

    GLint glMaxCombinedTextureImageUnits = 0;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &glMaxCombinedTextureImageUnits);
    std::cout << "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS = " << glMaxCombinedTextureImageUnits << std::endl;

    GLint glMaxVertexTextureImageUnits = 0;
    glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &glMaxVertexTextureImageUnits);
    std::cout << "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS = " << glMaxVertexTextureImageUnits << std::endl;

    GLint glMaxTextureSize = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &glMaxTextureSize);
    std::cout << "GL_MAX_TEXTURE_SIZE = " << glMaxTextureSize << std::endl;
   
    
    // Load the textures
    ::g_pTextureManager->SetBasePath("assets/textures");


//    ::g_pTextureManager->Create2DTextureFromBMPFile("2k_jupiter.bmp", true);
    ::g_pTextureManager->Create2DTextureFromBMPFile("Table_BaseColor.bmp", true);
    ::g_pTextureManager->Create2DTextureFromBMPFile("vurdalak_Base_Color.bmp", true);
    ::g_pTextureManager->Create2DTextureFromBMPFile("vurdalak_Roughness.bmp", true);
    ::g_pTextureManager->Create2DTextureFromBMPFile("glass.bmp", true);
    ::g_pTextureManager->Create2DTextureFromBMPFile("blood.bmp", true);
    ::g_pTextureManager->Create2DTextureFromBMPFile("plastic.bmp", true);
    ::g_pTextureManager->Create2DTextureFromBMPFile("rag.bmp", true);
    ::g_pTextureManager->Create2DTextureFromBMPFile("discard.bmp", true);
    ::g_pTextureManager->Create2DTextureFromBMPFile("noise.bmp", true);
    ::g_pTextureManager->Create2DTextureFromBMPFile("frosted.bmp", true);
    ::g_pTextureManager->Create2DTextureFromBMPFile("purple.bmp", true);
    ::g_pTextureManager->Create2DTextureFromBMPFile("Yellow_square.bmp", true);
    ::g_pTextureManager->Create2DTextureFromBMPFile("Green_square.bmp", true);
    ::g_pTextureManager->Create2DTextureFromBMPFile("water-ripple-texture.bmp", true);

    // Add a skybox texture
    std::string errorTextString;

    ::g_pTextureManager->SetBasePath("assets/textures/cubemaps");
    if (!::g_pTextureManager->CreateCubeTextureFromBMPFiles("Space01",
        "SpaceBox_right1_posX.bmp",     /* posX_fileName */
        "SpaceBox_left2_negX.bmp",      /*negX_fileName */
        "SpaceBox_top3_posY.bmp",       /*posY_fileName*/
        "SpaceBox_bottom4_negY.bmp",    /*negY_fileName*/
        "SpaceBox_front5_posZ.bmp",     /*posZ_fileName*/
        "SpaceBox_back6_negZ.bmp",      /*negZ_fileName*/
        true, errorTextString))
    {
        std::cout << "Didn't load because: " << errorTextString << std::endl;
    }
    else
    {
        std::cout << "Loaded the sunny day cube texture OK" << std::endl;
    }    
    if (!::g_pTextureManager->CreateCubeTextureFromBMPFiles("Indoors01",
        "posx.bmp",     /* posX_fileName */
        "negx.bmp",      /*negX_fileName */
        "negy.bmp",       /*posY_fileName*/
        "posy.bmp",    /*negY_fileName*/
        "posz.bmp",     /*posZ_fileName*/
        "negz.bmp",      /*negZ_fileName*/
        true, errorTextString))
    {
        std::cout << "Didn't load because: " << errorTextString << std::endl;
    }
    else
    {
        std::cout << "Loaded the Inddors cube texture OK" << std::endl;
    }    
    std::cout << "I've loaded these textures:" << std::endl;
    std::vector<std::string> vecTexturesLoaded;
    ::g_pTextureManager->GetLoadedTextureList(vecTexturesLoaded);

    for (std::vector<std::string>::iterator itTexName = vecTexturesLoaded.begin();
        itTexName != vecTexturesLoaded.end(); itTexName++)
    {
        std::cout << "\t" << *itTexName << std::endl;
    }


    // Create a skybox object (a sphere with inverted normals that moves with the camera eye)
    cMesh* pSkyBox = new cMesh();
    //pSkyBox->meshName = "Isosphere_Smooth_Normals.ply";
    //
    // We are using a sphere with INWARD facing normals. 
    // This is so we see the "back" of the sphere.
    // 
    pSkyBox->meshName = "Isosphere_Smooth_Inverted_Normals_for_SkyBox.ply";
    //
    // 2 main ways we can do a skybox:
    //
    // - Make a sphere really big, so everything fits inside
    //   (be careful of the far clipping plane)
    // 
    // - Typical way is:
    //   - Turn off the depth test
    //   - Turn off the depth function (i.e. writing to the depth buffer)
    //   - Draw the skybox object (which can be really small, since it's not interacting with the depth buffer)
    //   - Once drawn:
    //      - turn on the depth function
    //      - turn on the depth test
    // 
//    pSkyBox->scale = 5'000'000.0f;
    //    
    // We are now turning off the depth check, so this can be quite small, 
    // just big enough to be beyond the near clipping plane.
    // (here I'm making it 10x the size of the near plane)
    pSkyBox->scale = glm::vec3(::g_pFlyCamera->nearPlane * 1000.0f);
    // 
    pSkyBox->positionXYZ = ::g_pFlyCamera->getEye();


//    ::g_vec_pMeshes.push_back(pDebugSphere);     // Debug sphere is #5

    const double MAX_DELTA_TIME = 0.1;  // 100 ms
    double previousTime = glfwGetTime();

    /*{

        bool swapp = true;
        while (swapp)
        {
            swapp = false;
            for (int i = 0; i < ::g_vec_pMeshes.size() - 1; i++)
            {
                if (::g_vec_pMeshes[i]->alphaTransparency < ::g_vec_pMeshes[i + 1]->alphaTransparency)
                {
                    cMesh* temp = ::g_vec_pMeshes[i];
                    ::g_vec_pMeshes[i] = ::g_vec_pMeshes[i + 1];
                    ::g_vec_pMeshes[i + 1] = temp;
                    swapp = true;
                }
            }
        }
    }*/


    float offX = 0.f, offY = 0.f;

    LoadTheAnimationCommands(animationController);
    while (!glfwWindowShouldClose(pWindow))
    {
        plantManager.runThreads = true;
        animalManager.runThreads = true;
        float ratio;
        int width, height;
        glm::mat4 matModel;             // used to be "m"; Sometimes it's called "world"
        glm::mat4 matProjection;        // used to be "p";
        glm::mat4 matView;              // used to be "v";

        double currentTime = glfwGetTime();
//        double deltaTime = previousTime - currentTime;        OOOPSS!
        double deltaTime = currentTime - previousTime;
        deltaTime = (deltaTime > MAX_DELTA_TIME ? MAX_DELTA_TIME : deltaTime);
        previousTime = currentTime;
        //animationController->Update(deltaTime);

        glfwGetFramebufferSize(pWindow, &width, &height);
        ratio = width / (float)height;

        // Turn on the depth buffer
        glEnable(GL_DEPTH);         // Turns on the depth buffer
        glEnable(GL_DEPTH_TEST);    // Check if the pixel is already closer

        glViewport(0, 0, width, height);
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        // *******************************************************
        // Screen is cleared and we are ready to draw the scene...
        // *******************************************************

        // Update the title text
        glfwSetWindowTitle(pWindow, ::g_TitleText.c_str() );


        // Copy the light information into the shader to draw the scene
        ::g_pTheLights->CopyLightInfoToShader();

        // Place the "debug sphere" at the same location as the selected light (again)
        ::g_pDebugSphere->positionXYZ = ::g_pTheLights->theLights[0].position;
        // HACK: Debug sphere is 5th item added
//        ::g_vecMeshes[5].positionXYZ = gTheLights.theLights[0].position;

        matProjection = glm::perspective(
            ::g_pFlyCamera->FOV,       
            ratio,
            ::g_pFlyCamera->nearPlane,      // Near plane (as large as possible)
            ::g_pFlyCamera->farPlane);      // Far plane (as small as possible)

        //matProjection = glm::perspective(
        //    0.6f,       // in degrees
        //    ratio,
        //    10.0f,       // Near plane (as large as possible)
        //    1'000'000.0f);   // Far plane (as small as possible)

        ::g_pFlyCamera->Update(deltaTime);

        glm::vec3 cameraEye = ::g_pFlyCamera->getEye();
        glm::vec3 cameraAt = ::g_pFlyCamera->getAtInWorldSpace();
        glm::vec3 cameraUp = ::g_pFlyCamera->getUpVector();


        matView = glm::mat4(1.0f);
        matView = glm::lookAt( cameraEye,   // "eye"
                               cameraAt,    // "at"
                               cameraUp );

//        // HACK: Black hole discard example
//        ::g_blackHoleSize_for_discard_example += 1.0f;
//        GLint blackHoleSize_LocID = glGetUniformLocation(program, "blackHoleSize");
//        glUniform1f(blackHoleSize_LocID, ::g_blackHoleSize_for_discard_example);
//        std::cout << ::g_blackHoleSize_for_discard_example << std::endl;


//        glUniformMatrix4fv(matView_Location, 1, GL_FALSE, glm::value_ptr(matView));

        cShaderManager::cShaderProgram* pShaderProc = ::g_pShaderManager->pGetShaderProgramFromFriendlyName("Shader#1");

        glUniformMatrix4fv( pShaderProc->getUniformID_From_Name("matView"),
                            1, GL_FALSE, glm::value_ptr(matView));


        glUniformMatrix4fv(matProjection_Location, 1, GL_FALSE, glm::value_ptr(matProjection));


        
//        // Point the spotlight at the car
//        glm::vec3 vecToCar = pTheCar->positionXYZ - glm::vec3(::g_pTheLights->theLights[0].position);
//
//        vecToCar = glm::normalize (vecToCar);
//
//        ::g_pTheLights->theLights[0].direction = glm::vec4 (vecToCar, 1.0f);


        // *********************************************************************
        //    ____  _            ____             
        //   / ___|| | ___   _  | __ )  _____  __ 
        //   \___ \| |/ / | | | |  _ \ / _ \ \/ / 
        //    ___) |   <| |_| | | |_) | (_) >  <  
        //   |____/|_|\_\\__, | |____/ \___/_/\_\ 
        //               |___/                    
        //
        // Since this is a space game (most of the screen is filled with "sky"), 
        //  I'll draw the skybox first
        // 
        // This is updated to the "proper" way to do a skybox. 
        // Here's how:
        // 1. Turn off the depth function AND depth test
        // 2. Draw the skybox - it doesn't matter how big it is, since it's the 1st thing being drawn.
        //    Since I'm NOT writing to the depth buffer, it's written to the colour buffer 
        //    (i.e. we can see it), but anything drawn later doesn't "know" that's happened, 
        //    so all the other objects draw "on top" of it (on the colour buffer)
        // 
        GLint bIsSkyBox_LocID = glGetUniformLocation(program, "bIsSkyBox");
        glUniform1f(bIsSkyBox_LocID, (GLfloat)GL_TRUE);
        GLint bUseHeightMap_LodID = glGetUniformLocation(program, "bUseHeightMap");
        glUniform1f(bUseHeightMap_LodID, (GLfloat)GL_FALSE);
        
        glDisable( GL_DEPTH_TEST );
        // Move the "skybox object" with the camera
        pSkyBox->positionXYZ = ::g_pFlyCamera->getEye();
        DrawObject(
            pSkyBox, glm::mat4(1.0f),
            matModel_Location, matModelInverseTranspose_Location,
            program, ::g_pVAOManager);

        glUniform1f(bIsSkyBox_LocID, (GLfloat)GL_FALSE);
        //
        glEnable( GL_DEPTH_TEST );
        // *********************************************************************



        // Ordering the objects by transparency
        // 1. Make two arrays or vectors
        // 2. Clear them
        // 3. Go through all your "things to draw", separating transparent from non-transparent
        //    (if the alpha < 1.0f, then it's "transparent" )
        // 3a. glDisable(GL_BLEND);
        // 4. Draw all the NON transparent things (in any order you want)
        // 5. Sort ONLY the transparent object from "far" to "close" for THIS frame
        //     glm::distance() to compare 
        //     Can be ONE PASS of the bubble sort
        // 5a. glEnable(GL_BLEND);
        // 6. Draw transparent objects from FAR to CLOSE
    


        // Set up the discard texture, etc.

       /* {

            bool swapp = true;
            while (swapp)
            {
                swapp = false;
                for (int i = 0; i < ::g_vec_pMeshes.size() - 1; i++)
                {
                    if (distanceToCamera(::g_vec_pMeshes[i]->positionXYZ) < distanceToCamera(::g_vec_pMeshes[i+1]->positionXYZ))
                    {
                        cMesh* temp = ::g_vec_pMeshes[i];
                        ::g_vec_pMeshes[i] = ::g_vec_pMeshes[i + 1];
                        ::g_vec_pMeshes[i + 1] = temp;
                        swapp = true;
                    }
                }
            }
        }*/

        // **********************************************************************
        // Draw the "scene" of all objects.
        // i.e. go through the vector and draw each one...
        // **********************************************************************
        for (unsigned int index = 0; index != ::g_vec_pMeshes.size(); index++)
        {
            // So the code is a little easier...
             cMesh* pCurrentMesh = ::g_vec_pMeshes[index];

            matModel = glm::mat4(1.0f);  // "Identity" ("do nothing", like x1)
            //mat4x4_identity(m);

// ***************************************************
// HACK: Set the transparency for JUST the mobile shipyard...
            //{
            //    GLint bDiscardTransparencyWindowsOn_LodID = glGetUniformLocation(program, "bDiscardTransparencyWindowsOn");
            //    // Turn discard transparency off
            //    glUniform1f(bDiscardTransparencyWindowsOn_LodID, (GLfloat)GL_FALSE);

            //    //if (pCurrentMesh->friendlyName == "ShipYard")
            //    if (pCurrentMesh->friendlyName == "Terrain")
            //    {
            //        //                GLuint discardTextureNumber = ::g_pTextureManager->getTextureIDFromName("Lisse_mobile_shipyard-mal1.bmp");
            //        GLuint discardTextureNumber = ::g_pTextureManager->getTextureIDFromName("Banksy_Panda_Stencil.bmp");
            //        // I'm picking texture unit 30 since it's not in use.
            //        GLuint discardTextureUnit = 30;			// Texture unit go from 0 to 79
            //        glActiveTexture(discardTextureUnit + GL_TEXTURE0);	// GL_TEXTURE0 = 33984
            //        glBindTexture(GL_TEXTURE_2D, discardTextureNumber);
            //        GLint discardTexture_LocID = glGetUniformLocation(program, "discardTexture");
            //        glUniform1i(discardTexture_LocID, discardTextureUnit);

            //        // Turn discard function on
            //        glUniform1f(bDiscardTransparencyWindowsOn_LodID, (GLfloat)GL_TRUE);
            //    }
            //}
            // ***************************************************
            // ***************************************************
            // HACK: Set up a height map from a texture (used in the VERTEX shader)
            {
                GLint bUseHeightMap_LodID = glGetUniformLocation(program, "bUseHeightMap");
                glUniform1f(bUseHeightMap_LodID, (GLfloat)GL_FALSE);

                if (pCurrentMesh->friendlyName == "water")
                {

                    // I'm picking texture unit 31 since it's not in use.
                    GLuint heightMapTextureUnit = 31;			// Texture unit go from 0 to 79
                    glActiveTexture(heightMapTextureUnit + GL_TEXTURE0);	// GL_TEXTURE0 = 33984

                    GLuint heightMapTextureID = ::g_pTextureManager->getTextureIDFromName("noise.bmp");
                    glBindTexture(GL_TEXTURE_2D, heightMapTextureID);

                    GLint heightMapTexture_LocID = glGetUniformLocation(program, "heightMapTexture");
                    glUniform1i(heightMapTexture_LocID, heightMapTextureUnit);


                    GLint heightMapScale_LocID = glGetUniformLocation(program, "heightMapScale");
                    glUniform1f(heightMapScale_LocID, 0.8f);

                    GLint heightMapUVOffsetRotation_LocID = glGetUniformLocation(program, "heightMapUVOffsetRotation");
                    // Move to a global variable so we can move them with the keyboard
                    //glm::vec2 heightMapUVOffset = glm::vec2(0.0f, 0.0f);
                    glUniform3f(heightMapUVOffsetRotation_LocID,
                        ::g_heightMapUVOffsetRotation.x, ::g_heightMapUVOffsetRotation.y, ::g_heightMapUVOffsetRotation.z);

                    // Turn height map function on (will be turned off at next object - line 778)
                    glUniform1f(bUseHeightMap_LodID, (GLfloat)GL_TRUE);


                    GLint heightMapOffset_ID = glGetUniformLocation(program, "heightMapOffset");
                    glUniform2f(heightMapOffset_ID, offX, offY);
                    offX += 0.05f * deltaTime;
                    offY += 0.05f * deltaTime;
//                    std::cout << "heightMapTextureID = " << heightMapTextureID << std::endl;
//                    std::cout << "heightMapTextureUnit = " << heightMapTextureUnit << std::endl;
//                    std::cout << "heightMapTextureUnit + GL_TEXTURE0 = " << heightMapTextureUnit + GL_TEXTURE0 << std::endl;
//
//                    // returns a single value indicating the active multitexture unit. The initial value is GL_TEXTURE0.
//                    GLint glActiveTexture = 0;
//                    glGetIntegerv(GL_ACTIVE_TEXTURE, &glActiveTexture);
//                    std::cout << "GL_ACTIVE_TEXTURE = " << glActiveTexture << std::endl;
//                    std::cout << "GL_ACTIVE_TEXTURE - GL_TEXTURE0 = " << glActiveTexture - GL_TEXTURE0 << std::endl;
//
//                    // returns a single value, the name of the texture currently bound to the target GL_TEXTURE_2D. The initial value is 0.
//                    GLint glTextureBindings = 0;
//                    glGetIntegerv(GL_TEXTURE_BINDING_2D, &glTextureBindings);
//                    std::cout << "GL_TEXTURE_BINDING_2D = " << glTextureBindings << std::endl;

                }//if (pCurrentMesh->friendlyName == "FlatTerrain")
            }
// ***************************************************

            DrawObject(pCurrentMesh,
                matModel,
                matModel_Location,
                matModelInverseTranspose_Location,
                program,
                ::g_pVAOManager);


        }//for (unsigned int index
        // Scene is drawn
        // **********************************************************************



        glUniform1f(bUseHeightMap_LodID, (GLfloat)GL_FALSE);
        DrawDebugObjects(matModel_Location, matModelInverseTranspose_Location, program, ::g_pVAOManager);


        // "Present" what we've drawn.
        glfwSwapBuffers(pWindow);        // Show what we've drawn

        // Process any events that have happened
        glfwPollEvents();

        // Handle OUR keyboard, mouse stuff
        handleAsyncKeyboard(pWindow, deltaTime);
        handleAsyncMouse(pWindow, deltaTime);

    }//while (!glfwWindowShouldClose(window))

    // All done, so delete things...
    ::g_ShutDown(pWindow);

    delete animationController;

    glfwDestroyWindow(pWindow);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}





void DrawDebugObjects(
        GLint matModel_Location,
        GLint matModelInverseTranspose_Location,
        GLuint program,
        cVAOManager* pVAOManager)
{


    {   // Draw a sphere where the camera is looking.
        // Take the at - eye --> vector 
        glm::vec3 eye = ::g_pFlyCamera->getEye();
        glm::vec3 at = ::g_pFlyCamera->getAtInWorldSpace();

        glm::vec3 deltaDirection = at - eye;
        // Normalize to make this vector 1.0 units in length
        deltaDirection = glm::normalize(deltaDirection);

        float SphereDistanceFromCamera = 300.0f;
        glm::vec3 sphereLocation =
            eye + (deltaDirection * SphereDistanceFromCamera);

        // Draw the sphere

        ::g_pDebugSphere->positionXYZ = sphereLocation;
        ::g_pDebugSphere->scale = glm::vec3(5.f);
        ::g_pDebugSphere->objectDebugColourRGBA = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);

        DrawObject(::g_pDebugSphere,
            glm::mat4(1.0f),
            matModel_Location,
            matModelInverseTranspose_Location,
            program,
            ::g_pVAOManager);

    }//Draw a sphere where the camera is looking.



    if ( ::g_bShowDebugShere )
    {
        // Draw other things, like debug objects, UI, whatever
        glm::mat4 matModelDS = glm::mat4(1.0f);  // "Identity" ("do nothing", like x1)

        // Draw a small white shere at the location of the light
        sLight& currentLight = ::g_pTheLights->theLights[::g_selectedLight];

        ::g_pDebugSphere->positionXYZ = currentLight.position;
        ::g_pDebugSphere->scale = glm::vec3(1.f);
        ::g_pDebugSphere->objectDebugColourRGBA = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        DrawObject(::g_pDebugSphere,
                   matModelDS,
                   matModel_Location,
                   matModelInverseTranspose_Location,
                   program,
                   ::g_pVAOManager);

        // Save old debug sphere model name
        std::string oldDebugSphereModel = ::g_pDebugSphere->meshName;

        const float LOW_RES_SPHERE_DISTANCE = 50.0f;
        const std::string LOW_RES_SPHERE_MODEL = "ISO_Shphere_flat_3div_xyz_n_rgba_uv.ply";
        const std::string HIGH_RES_SPHERE_MODEL = "ISO_Shphere_flat_4div_xyz_n_rgba_uv.ply";

        //float calcApproxDistFromAtten( 
        //      float targetLightLevel, 
        //      float accuracy, 
        //      float infiniteDistance, 
        //      float constAttenuation, 
        //      float linearAttenuation,  
        //      float quadraticAttenuation, 
        //	    unsigned int maxIterations = DEFAULTMAXITERATIONS /*= 50*/ );

                // How far away is 95% brightness?
        float distTo95Percent = ::g_pTheLights->lightHelper.calcApproxDistFromAtten(0.95f,    /* the target light level I want*/
                                                                          0.01f,    /*accuracy - how close to 0.25f*/
                                                                          10000.0f, /*infinity away*/
                                                                          currentLight.atten.x, /*const atten*/
                                                                          currentLight.atten.y, /*linear atten*/
                                                                          currentLight.atten.z, /*quadratic atten*/
                                                                          cLightHelper::DEFAULTMAXITERATIONS);
        // Draw a red sphere at 95%
        ::g_pDebugSphere->scale = glm::vec3(distTo95Percent);
        ::g_pDebugSphere->objectDebugColourRGBA = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

        ::g_pDebugSphere->meshName = (::g_pDebugSphere->scale.x < LOW_RES_SPHERE_DISTANCE ? LOW_RES_SPHERE_MODEL : HIGH_RES_SPHERE_MODEL);

        DrawObject(::g_pDebugSphere,
                   matModelDS,
                   matModel_Location,
                   matModelInverseTranspose_Location,
                   program,
                   ::g_pVAOManager);

        // How far away is 50% brightness?
        float distTo50Percent = ::g_pTheLights->lightHelper.calcApproxDistFromAtten(0.50f,    /* the target light level I want*/
                                                                          0.01f,    /*accuracy - how close to 0.25f*/
                                                                          10000.0f, /*infinity away*/
                                                                          currentLight.atten.x, /*const atten*/
                                                                          currentLight.atten.y, /*linear atten*/
                                                                          currentLight.atten.z, /*quadratic atten*/
                                                                          cLightHelper::DEFAULTMAXITERATIONS);
        // Draw a red sphere at 50%
        ::g_pDebugSphere->scale = glm::vec3(distTo50Percent);
        ::g_pDebugSphere->objectDebugColourRGBA = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
        ::g_pDebugSphere->meshName = (::g_pDebugSphere->scale.x < LOW_RES_SPHERE_DISTANCE ? LOW_RES_SPHERE_MODEL : HIGH_RES_SPHERE_MODEL);
        DrawObject(::g_pDebugSphere,
                   matModelDS,
                   matModel_Location,
                   matModelInverseTranspose_Location,
                   program,
                   ::g_pVAOManager);

        // How far away is 25% brightness?
        float distTo25Percent = ::g_pTheLights->lightHelper.calcApproxDistFromAtten(0.25f,    /* the target light level I want*/
                                                                          0.01f,    /*accuracy - how close to 0.25f*/
                                                                          10000.0f, /*infinity away*/
                                                                          currentLight.atten.x, /*const atten*/
                                                                          currentLight.atten.y, /*linear atten*/
                                                                          currentLight.atten.z, /*quadratic atten*/
                                                                          cLightHelper::DEFAULTMAXITERATIONS);
        // Draw a red sphere at 25%
        ::g_pDebugSphere->scale = glm::vec3(distTo25Percent);
        ::g_pDebugSphere->objectDebugColourRGBA = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
        ::g_pDebugSphere->meshName = (::g_pDebugSphere->scale.x < LOW_RES_SPHERE_DISTANCE ? LOW_RES_SPHERE_MODEL : HIGH_RES_SPHERE_MODEL);
        DrawObject(::g_pDebugSphere,
                   matModelDS,
                   matModel_Location,
                   matModelInverseTranspose_Location,
                   program,
                   ::g_pVAOManager);

        // How far away is 5% brightness?
        float distTo5Percent = ::g_pTheLights->lightHelper.calcApproxDistFromAtten(0.05f,    /* the target light level I want*/
                                                                         0.01f,    /*accuracy - how close to 0.25f*/
                                                                         10000.0f, /*infinity away*/
                                                                         currentLight.atten.x, /*const atten*/
                                                                         currentLight.atten.y, /*linear atten*/
                                                                         currentLight.atten.z, /*quadratic atten*/
                                                                         cLightHelper::DEFAULTMAXITERATIONS);
        // Draw a red sphere at 5%
        ::g_pDebugSphere->scale = glm::vec3(distTo5Percent);
        ::g_pDebugSphere->objectDebugColourRGBA = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
        ::g_pDebugSphere->meshName = (::g_pDebugSphere->scale.x < LOW_RES_SPHERE_DISTANCE ? LOW_RES_SPHERE_MODEL : HIGH_RES_SPHERE_MODEL);
        DrawObject(::g_pDebugSphere,
                   matModelDS,
                   matModel_Location,
                   matModelInverseTranspose_Location,
                   program,
                   ::g_pVAOManager);


        ::g_pDebugSphere->meshName = oldDebugSphereModel;

    }//if ( ::g_bShowDebugShere )

    return;
}
