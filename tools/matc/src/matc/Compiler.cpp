/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Compiler.h"

#include <filaflat/ChunkContainer.h>
#include <matdbg/ShaderExtractor.h>
#include <matdbg/ShaderInfo.h>
#include <matdbg/JsonWriter.h>

#include <iostream>
#include <fstream>
#include <iomanip>

using namespace filamat;

namespace matc {

bool Compiler::writeBlob(const Package &pkg, const Config& config) const noexcept {
    Config::Output* output = config.getOutput();
    if (!output->open()) {
        std::cerr << "Unable to create blob file." << std::endl;
        return false;
    }
    // to bgfx material
	using namespace filament::matdbg;
	filaflat::ChunkContainer container(pkg.getData(), pkg.getSize());
	if (!container.parse()) {
		return false;
	}

	JsonWriter jwriter;
	if (!jwriter.writeMaterialInfo(container)) {
		return false;
	}
	//std::cout << jwriter.getJsonString();
    uint32_t jsonsize = jwriter.getJsonSize();
    output->write((uint8_t*)&jsonsize, sizeof(uint32_t));
    output->write((const uint8_t*)jwriter.getJsonString(), jsonsize);
    uint32_t shaderoffset = sizeof(uint32_t) + jsonsize;
	
	std::vector<ShaderInfo> info;
	ShaderExtractor parser(filament::backend::Backend::OPENGL, pkg.getData(), pkg.getSize());
	if (!parser.parse()) {
		return false;
	}

	info.resize(getShaderCount(container, filamat::ChunkType::MaterialGlsl));
	if (!getGlShaderInfo(container, info.data())) {
		std::cerr << "Failed to parse GLSL chunk." << std::endl;
		return false;
	}

    uint32_t shadercount = info.size();
    output->write((uint8_t*)&shadercount, sizeof(uint32_t));
    shaderoffset += sizeof(uint32_t);

    std::vector<filaflat::ShaderContent> contents;
    contents.resize(shadercount);
    shaderoffset += sizeof(uint32_t) * 3 * shadercount;
	for (int i = 0; i < shadercount; i++) {
		const auto& item = info[i];
        uint32_t shaderkey = (uint32_t(item.shaderModel) << 16) | (uint32_t(item.pipelineStage) << 8) | item.variant.key;
        output->write((uint8_t*)&shaderkey, sizeof(uint32_t));
        output->write((uint8_t*)&shaderoffset, sizeof(uint32_t));
        parser.getShader(item.shaderModel, item.variant, item.pipelineStage, contents[i]);
        uint32_t contentsize = contents[i].size();
        output->write((uint8_t*)&contentsize, sizeof(uint32_t));
        shaderoffset += contentsize;
	}

    for (int i = 0; i < shadercount; i++) {
        output->write(contents[i].data(), contents[i].size());
    }

//     output->write(pkg.getData(), pkg.getSize());

    output->close();

    return true;
}

bool Compiler::writeBlobAsHeader(const Package &pkg, const Config& config) const noexcept {
    uint8_t* data = pkg.getData();

    Config::Output* output = config.getOutput();
    if (!output->open()) {
        std::cerr << "Unable to create header file." << std::endl;
        return false;
    }

    std::ostream& file = output->getOutputStream();
    if (config.isDebug()) {
        file << "// This file was generated with the following command:" << std::endl;
        file << "// " << config.toString() << " ";
        file << std::endl;

        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        file << "// Created: " << std::put_time(&tm, "%Y-%m-%d at %H:%M:%S") << std::endl;
    }

    size_t i = 0;
    for ( ; i < pkg.getSize(); i++) {
        if (i > 0 && i % 20 == 0) {
            file << std::endl;
        }
        file << "0x" << std::setfill('0') << std::setw(2) << std::hex << (int) data[i] << ", ";
    }

    if (i % 20 != 0) file << std::endl;
    file << std::endl;

    output->close();

    return true;
}

} // namespace matc
