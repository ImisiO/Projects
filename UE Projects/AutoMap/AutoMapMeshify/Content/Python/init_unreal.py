import unreal
import glob
import os

def LogInit():
    print('Auto Map Meshify Plugin Python Init Initialized')

def DeleteOldFilesInDirectory(directory):
    if not os.path.exists(directory):
        print(f"The directory {directory} does not exist.")
        return

    # Ensure the provided path is a directory
    if not os.path.isdir(directory):
        print(f"The path {directory} is not a directory.")
        return

    for fileName in os.listdir(directory):
        if not fileName.lower().endswith('.fbx'):
            continue

        filePath = os.path.join(directory, fileName)
        try:
            # Check if it is a file (not a directory)
            if os.path.isfile(filePath) or os.path.islink(filePath):
                print(f"Deleting old FBX file: {fileName}")
                os.unlink(filePath)
        except Exception as e:
            print(f"Failed to delete {filePath}. Reason: {e}")

def DeleteOldFBXFiles():
    assetPathManagerESS = unreal.get_editor_subsystem(unreal.AssetPathSubsystem)
    
    # Getting file paths
    ueGeneratedTerrainPath = assetPathManagerESS.get_ue_generated_terrain_meshes_abs_path()
    ueGeneratedAccentPath = assetPathManagerESS.get_ue_generated_accent_meshes_abs_path()
    houdiniGeneratedTerrainPath = assetPathManagerESS.get_imported_terrain_meshes_abs_path()
    houdiniGeneratedAccentPath = assetPathManagerESS.get_imported_accent_meshes_abs_path()

    DeleteOldFilesInDirectory(ueGeneratedTerrainPath)
    DeleteOldFilesInDirectory(ueGeneratedAccentPath)
    DeleteOldFilesInDirectory(houdiniGeneratedTerrainPath)
    DeleteOldFilesInDirectory(houdiniGeneratedAccentPath)

def GetMeshAssets(dirOfAssets):
    assetReg = unreal.AssetRegistryHelpers.get_asset_registry()

    assetData = assetReg.get_assets_by_path(dirOfAssets)

    print('Number of assets in ' + dirOfAssets + ' is ' + str(len(assetData)))

    meshAssets = []
    for asset in assetData:
        if asset.asset_class_path.asset_name  == 'StaticMesh':
            meshAsset = asset.get_asset()
            meshAssets.append(meshAsset)

    print('Number of mesh assets: '+ str(len(meshAssets)))

    return meshAssets

def GetUEGeneratedTerrainMeshAssets():
    print('Getting UE Generated Terrain Mesh Assets')
    
    assetPathManagerESS = unreal.get_editor_subsystem(unreal.AssetPathSubsystem)
    ueGeneratedTerrainPath = assetPathManagerESS.get_ue_generated_terrain_meshes_game_path()

    terrainMeshAssets = GetMeshAssets(ueGeneratedTerrainPath)
    print('---------------------------')

    return terrainMeshAssets

def GetUEGeneratedAccentMeshAssets():
    print('Getting UE Generated Accent Mesh Assets')

    assetPathManagerESS = unreal.get_editor_subsystem(unreal.AssetPathSubsystem)
    ueGeneratedAccentPath = assetPathManagerESS.get_ue_generated_accent_meshes_game_path()

    accentMeshAssets = GetMeshAssets(ueGeneratedAccentPath)
    print('---------------------------')

    return accentMeshAssets

def GetImportedTerrainMeshAssets():
    print('Getting Houdini Generated Terrain Mesh Assets')
    
    assetPathManagerESS = unreal.get_editor_subsystem(unreal.AssetPathSubsystem)
    terrainImportPath = assetPathManagerESS.get_imported_terrain_meshes_game_path()

    importedTerrainMeshAssets = GetMeshAssets(terrainImportPath)
    print('---------------------------')

    return importedTerrainMeshAssets

def GetImportedAccentMeshAssets():
    print('Getting Houdini Generated Accent Mesh Assets')
    
    assetPathManagerESS = unreal.get_editor_subsystem(unreal.AssetPathSubsystem)
    accentImportPath = assetPathManagerESS.get_imported_accent_meshes_game_path()

    importedAccentMeshAssets = GetMeshAssets(accentImportPath)
    print('---------------------------')

    return importedAccentMeshAssets

def ExportMapAssets():
    assetPathManagerESS = unreal.get_editor_subsystem(unreal.AssetPathSubsystem)
    
    terrainMeshAssets = GetUEGeneratedTerrainMeshAssets()
    accentMeshAssets = GetUEGeneratedAccentMeshAssets()

    numberOfAssets = len(terrainMeshAssets) + len(accentMeshAssets)

    dialogLabel = 'Exporting UE Generate Auto Map Meshes to FBX'

    # Running bool so if task is exited early we can exit process
    isRunning = True

    with unreal.ScopedSlowTask(numberOfAssets , dialogLabel) as slow_task:
        slow_task.make_dialog(True)

        for terrainAsset in terrainMeshAssets:
            if slow_task.should_cancel():
                isRunning = False
                break

            assetName = terrainAsset.get_name()

            exportTask = unreal.AssetExportTask()
            exportTask.automated = True

            fileNameWithExt = assetName + '_ue_ex' + '.fbx'

            filePath = assetPathManagerESS.get_ue_generated_terrain_meshes_abs_path()

            exportTask.filename = filePath + fileNameWithExt
            exportTask.prompt = False
            exportTask.object = terrainAsset

            print(exportTask.filename)

            if isinstance(terrainAsset, unreal.StaticMesh):

                exportTask.options = unreal.FbxExportOption()
                exportTask.options.set_editor_property('level_of_detail', False)
                fbxExporter = unreal.StaticMeshExporterFBX()
                exportTask.exporter = fbxExporter
                fbxExporter.run_asset_export_task(exportTask)
                slow_task.enter_progress_frame(1)
        
        for accentAsset in accentMeshAssets:

            # Cancelling process if user presses cancel
            if slow_task.should_cancel():
                isRunning = False
                break
            
            # if we cancel the export process dont export accent asset
            if isRunning != True:
                break

            assetName = accentAsset.get_name()

            exportTask = unreal.AssetExportTask()
            exportTask.automated = True

            fileNameWithExt = assetName + '_ue_ex' + '.fbx'
        
            filePath = assetPathManagerESS.get_ue_generated_accent_meshes_abs_path()

            exportTask.filename = filePath + fileNameWithExt
            exportTask.prompt = False
            exportTask.object = accentAsset

            print(exportTask.filename)

            if isinstance(accentAsset, unreal.StaticMesh):
                exportTask.options = unreal.FbxExportOption()
                exportTask.options.set_editor_property('level_of_detail', False)
                fbxExporter = unreal.StaticMeshExporterFBX()
                exportTask.exporter = fbxExporter
                fbxExporter.run_asset_export_task(exportTask)
                slow_task.enter_progress_frame(1)

def GetHouidiniExportFiles(mapElementType):
    assetPathManagerESS = unreal.get_editor_subsystem(unreal.AssetPathSubsystem)

    dirPath = ''
    
    if mapElementType == 'T':
        dirPath = assetPathManagerESS.get_imported_terrain_meshes_abs_path()
    elif mapElementType == 'A':
        dirPath = assetPathManagerESS.get_imported_accent_meshes_abs_path()

    objFiles = [f.replace('\\', '/') for f in glob.glob(dirPath + '*.obj')]
    
    print(objFiles)

    return objFiles

def BuildImportTask(filename, destination_path, options=None):
    task = unreal.AssetImportTask()
    task.set_editor_property('automated', True)
    task.set_editor_property('destination_name', '')
    task.set_editor_property('destination_path', destination_path)
    task.set_editor_property('filename', filename)
    task.set_editor_property('replace_existing', True)
    task.set_editor_property('save', True)
    task.set_editor_property('options', options)
    return task

def BuildStaticMeshImportOptions():
    options = unreal.FbxImportUI()

    options.set_editor_property('import_mesh', True)
    options.set_editor_property('import_textures', False)
    options.set_editor_property('import_materials', False)
    options.set_editor_property('import_as_skeletal', False)
    
    options.static_mesh_import_data.set_editor_property('normal_import_method', unreal.FBXNormalImportMethod.FBXNIM_COMPUTE_NORMALS)
    options.static_mesh_import_data.set_editor_property('auto_generate_collision', True)
    options.static_mesh_import_data.set_editor_property('generate_lightmap_u_vs', False)
    options.static_mesh_import_data.set_editor_property('import_mesh_lo_ds', False)
    options.static_mesh_import_data.set_editor_property('combine_meshes', True)
    return options

def RecomputeNormalsForImportedMeshes():
    staticMeshEditorSS = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
   
    importedTerrainMeshAssets = GetImportedTerrainMeshAssets()
    importedAccentMeshAssets = GetImportedAccentMeshAssets()

    numberOfAssets = len(importedTerrainMeshAssets) + len(importedAccentMeshAssets) 
    dialogLabel = 'Recomputing Normals For Imported Meshes'

    # Running bool so if task is exited early we can exit process
    isRunning = True

    with unreal.ScopedSlowTask(numberOfAssets , dialogLabel) as slow_task:
        slow_task.make_dialog(True)

        for terrainAsset in importedTerrainMeshAssets:

            for lodIndex in range(terrainAsset.get_num_lods()):

                # Cancelling process if user presses cancel
                if slow_task.should_cancel():
                    isRunning = False
                    break

                # Recompute normals of mesh as sometimes normals are broken after importing
                staticMeshBuildSettings = staticMeshEditorSS.get_lod_build_settings(terrainAsset, lodIndex)
                staticMeshBuildSettings.recompute_normals = True
                staticMeshEditorSS.set_lod_build_settings(terrainAsset, lodIndex, staticMeshBuildSettings)

            terrainAsset.modify()
            slow_task.enter_progress_frame(1)
        
        # Only continue if the user has not cancelled the process
        if isRunning == True:
            for accentAsset in importedAccentMeshAssets:
                
                for lodIndex in range(accentAsset.get_num_lods()):

                    staticMeshBuildSettings = staticMeshEditorSS.get_lod_build_settings(accentAsset, lodIndex)
                    staticMeshBuildSettings.recompute_normals = True
                    staticMeshEditorSS.set_lod_build_settings(accentAsset, lodIndex, staticMeshBuildSettings)
                
                accentAsset.modify()
                slow_task.enter_progress_frame(1)

def ImportHoudiniMeshExports():
    assetPathManagerESS = unreal.get_editor_subsystem(unreal.AssetPathSubsystem)

    terrainFileNames = GetHouidiniExportFiles('T')
    accentFileNames = GetHouidiniExportFiles('A')

    print(terrainFileNames)
    print(accentFileNames)

    terrainImportPath = assetPathManagerESS.get_imported_terrain_meshes_game_path()
    accentImportPath = assetPathManagerESS.get_imported_accent_meshes_game_path()

    tasks = []

    for terrainFile in terrainFileNames:
        task = BuildImportTask(terrainFile, terrainImportPath, BuildStaticMeshImportOptions())
        print('Creating Import Task (Terrain) for' + terrainFile)
        tasks.append(task)

    for accentFile in accentFileNames:
        task = BuildImportTask(accentFile, accentImportPath, BuildStaticMeshImportOptions())
        print('Creating Import Task (Accents) for' + accentFile)
        tasks.append(task)
        
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)

def ExportUEGeneratedMeshes():

    #DeleteOldFBXFiles()
    ExportMapAssets()     

LogInit()