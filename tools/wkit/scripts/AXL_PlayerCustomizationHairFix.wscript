import * as Logger from 'Logger.wscript'
import { CName } from 'TypeHelper.wscript'
import * as TypeHelper from 'TypeHelper.wscript'

const config = JSON.parse(wkit.YamlToJson(wkit.LoadRawJsonFromProject('PlayerCustomizationAlias.xl', 'json')))

const affectedHairMeshes = [
  ...config['resource']['alias']['player_ma_hair.mesh'],
  ...config['resource']['alias']['player_ma_hat_hair.mesh'],
  ...config['resource']['alias']['player_wa_hair.mesh'],
  ...config['resource']['alias']['player_wa_hat_hair.mesh'],
]

const logInfo = false
const logWarnings = true
const generateExtensionJson = true
const generateFixedResources = false

if (logInfo || logWarnings) {
  Logger.Info('Converting hair meshes...')
}

const usedMasterMaterials = new Set()
const extensionData = {}

for (let meshPath of affectedHairMeshes) {
  let meshFile = wkit.GetFileFromBase(meshPath)

  if (!meshFile) {
    Logger.Error(`Mesh cannot be opened: ${meshPath}`)
    continue
  }

  let meshJson = wkit.GameFileToJson(meshFile)

  if (!meshJson) {
    Logger.Error(`Mesh cannot be parsed: ${meshPath}`)
    continue
  }

  let meshRaw = TypeHelper.JsonParse(meshJson)
  let meshInstance = meshRaw['Data']['RootChunk']

  let contextParams = {}
  let materialMappings = {}

  if (logInfo || logWarnings) {
    Logger.Info(`${meshPath}`)

    if (logInfo) {
      Logger.Info('  Materials:')
    }
  }

  for (let materialEntry of meshInstance['materialEntries']) {
    const materialOldName = materialEntry['name'].toString()

    if (materialOldName === 'hologram') {
      if (materialEntry['isLocalInstance']) {
        materialEntry['index']++ // compensate for @context material entry
      }
      continue
    }

    let materialData = {}
    let materialInstance
    if (materialEntry['isLocalInstance']) {
      materialInstance = meshInstance['localMaterialBuffer']['materials'][materialEntry['index']]
      materialEntry['index']++ // compensate for @context material entry
    } else {
      let materialPath = meshInstance['externalMaterials'][materialEntry['index']]['DepotPath'].toString()
      materialInstance = loadMaterialInstace(materialPath)
    }

    collectMaterialData(materialInstance, materialData, contextParams)

    if (!materialData['template']) {
      continue
    }

    materialEntry['name'] = new CName(materialData['name'] + materialData['template'])
    materialMappings[materialOldName] = materialEntry['name']

    if (logInfo) {
      Logger.Info(`  - ${materialEntry['name']}`)
    }

    if (materialData['master']) {
      usedMasterMaterials.add(materialData['master'])
    }
  }

  if (logInfo) {
    Logger.Info('  Context:')
    for (let paramName in contextParams) {
      Logger.Info(`  ${paramName} = ${contextParams[paramName]}`)
    }
  }

  if (generateFixedResources) {
    const contextInstance = {
      '$type': 'CMaterialInstance',
      'values': []
    }

    for (let paramName in contextParams) {
      contextInstance.values.push({
        '$type': 'CName',
        [paramName]: {
          '$type': 'CName',
          '$storage': 'string',
          '$value': contextParams[paramName],
        }
      })
    }

    if (!meshInstance['localMaterialBuffer']['materials']) {
      meshInstance['localMaterialBuffer']['materials'] = []
    }

    meshInstance['localMaterialBuffer']['materials'].unshift(contextInstance)
    meshInstance['materialEntries'].unshift({
      '$type': 'CMeshMaterialEntry',
      'index': 0,
      'isLocalInstance': 1,
      'name': {
        '$type': 'CName',
        '$storage': 'string',
        '$value': '@context'
      }
    })

    const unmappedMaterials = new Set()
    for (let i = 0, n = meshInstance['appearances'].length; i < n; ++i) {
      const chunkMaterials = meshInstance['appearances'][i]['Data']['chunkMaterials']
      for (let j = 0, m = chunkMaterials.length; j < m; ++j) {
        if (materialMappings.hasOwnProperty(chunkMaterials[j])) {
          chunkMaterials[j] = materialMappings[chunkMaterials[j]]
        } else {
          if (!unmappedMaterials.has(chunkMaterials[j].toString())) {
            unmappedMaterials.add(chunkMaterials[j].toString())
            if (logInfo || logWarnings) {
              (i === 0 ? Logger.Error : Logger.Warning)(`No mapping for material: ${chunkMaterials[j]}`)
            }
          }
        }
      }
    }

    meshJson = TypeHelper.JsonStringify(meshRaw)
    wkit.SaveToProject(meshPath, wkit.JsonToCR2W(meshJson))
  }

  if (generateExtensionJson) {
    const nameMappings = {}
    for (const materialName in materialMappings) {
      if (materialMappings.hasOwnProperty(materialName)) {
        nameMappings[materialName] = materialMappings[materialName].toString()
      }
    }
    extensionData[meshPath] = {
      names: nameMappings,
      context: contextParams,
    }
  }
}

if (logInfo || logWarnings) {
  Logger.Info('Used master materials:')
  for (let materialPath of usedMasterMaterials) {
    Logger.Info(`- ${materialPath}`)
  }

  Logger.Success('Converion finished.')
}

if (generateExtensionJson) {
  wkit.SaveToRaw('PlayerCustomizationHairFix.xl', wkit.JsonToYaml(JSON.stringify({resource: {fix: extensionData}})))
}

function collectMaterialData(materialInstance, materialData, contextParams) {
  let basePath = materialInstance['baseMaterial']['DepotPath'].toString()
  if (!basePath.match(/\.(?:re)?mt$/)) {
    let baseInstance = loadMaterialInstace(basePath)

    collectMaterialData(baseInstance, materialData, contextParams)

    let masterPathMatch = basePath.match(/^base\\characters\\common\\hair\\textures\\hair_profiles\\_master__([a-z]+)(?:_bright)?_?\.mi$/)
    if (masterPathMatch) {
      materialData['template'] = '@' + masterPathMatch[1]
      materialData['master'] = basePath
    }
  }

  for (let materialParam of materialInstance['values']) {
    if (materialParam.hasOwnProperty('HairProfile')) {
      let profilePath = materialParam['HairProfile']['DepotPath'].toString()
      let profileMatch = profilePath.match(/\\([^\\]+)\.hp$/)
      if (profileMatch) {
        materialData['name'] = profileMatch[1]
      }
      continue
    }

    if (materialParam.hasOwnProperty('DiffuseTexture')) {
      let texturePath = materialParam['DiffuseTexture']['DepotPath'].toString()
      if (texturePath && !texturePath.match(/^engine\\textures\\editor\\/)) {
        contextParams['CapDiffuseTexture'] = texturePath
        materialData['template'] = '@cap'
      }
      continue
    }

    if (materialParam.hasOwnProperty('MaskTexture')) {
      let texturePath = materialParam['MaskTexture']['DepotPath'].toString()
      if (texturePath && !texturePath.match(/^engine\\textures\\editor\\/)) {
        contextParams['CapMaskTexture'] = texturePath
        materialData['template'] = '@cap'
      }
      continue
    }

    if (materialParam.hasOwnProperty('SecondaryMask')) {
      let texturePath = materialParam['SecondaryMask']['DepotPath'].toString()
      if (texturePath && !texturePath.match(/^engine\\textures\\editor\\/)) {
        contextParams['CapSecondaryMaskTexture'] = texturePath
        materialData['template'] = '@cap01'
      }
      continue
    }

    if (materialParam.hasOwnProperty('GradientMap')) {
      let gradientPath = materialParam['GradientMap']['DepotPath'].toString()
      let gradientMatch = gradientPath.match(/^base\\characters\\common\\hair\\textures\\cap_gradiants\\hh_cap_grad__([^.]+)\.xbm$/)
      if (gradientMatch) {
        materialData['name'] = gradientMatch[1]
      }
    }
  }
}

function loadMaterialInstace(materialPath) {
  let materialFile = wkit.GetFileFromBase(materialPath)
  let materialRaw = TypeHelper.JsonParse(wkit.GameFileToJson(materialFile))
  return materialRaw['Data']['RootChunk']
}
