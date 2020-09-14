#include "Vimgui.h"

Demo::VimguiCore::VimguiCore(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, uint32_t swapWidth, uint32_t swapHeight, VkRenderPass renderPass, uint32_t graphicsQueueIndex, VkQueue graphicsQueue)
{
	this->physicalDevice = physicalDevice;
	this->logicalDevice = logicalDevice;
	this->swapchainExtentWidth = swapWidth;
	this->swapchainExtentHeight = swapHeight;
	this->renderPass = renderPass;
	this->graphicsQueueIndex = graphicsQueueIndex;
	this->graphicsQueue = graphicsQueue;
	prepare();
	prepared = true;
}

void Demo::VimguiCore::prepare()
{
	prepUniformBuffers();
	cqPrepPipelineCache();
	cqPrepVertexInputDescription();
	cqPrepDescriptorSetLayout();
	cqPrepPipelineLayout();
	cqPrepPipeline();
	cqPrepDescriptorPool();
	cqPrepDescriptorSet();
	cqPrepColorQuadBuffer();

	parseFontFile("assets\\fonts\\sdf_arial");
	loadImagea("assets\\fonts\\sdf_arial.png");
	dftPrepPipelineCache();
	dftPrepVertexInputDescription();
	dftPrepDescriptorSetLayout();
	dftPrepPipelineLayout();
	dftPrepPipeline();
	dftPrepDescriptorPool();
	dftPrepDescriptorSet();
	dftPrepDistanceFieldTextBuffer();

	clPrepPipelineCache();
	clPrepVertexInputDescription();
	clPrepDescriptorSetLayout();
	clPrepPipelineLayout();
	clPrepPipeline();
	clPrepDescriptorPool();
	clPrepDescriptorSet();
	clPrepColorLineBuffer();
}

Demo::VimguiCore::~VimguiCore()
{
	vkDestroyPipeline(logicalDevice, cqPipeline, nullptr);
	vkDestroyPipelineCache(logicalDevice, cqPipelineCache, nullptr);
	vkDestroyPipelineLayout(logicalDevice, cqPipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(logicalDevice, cqDescriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(logicalDevice, cqDescriptorPool, nullptr);
	cqBuffer->unmap();
	delete cqBuffer;


	vkDestroyPipeline(logicalDevice, dftNormalPipeline, nullptr);
	vkDestroyPipeline(logicalDevice, dftOutlinePipeline, nullptr);
	vkDestroyPipeline(logicalDevice, dftDropshadowPipeline, nullptr);
	vkDestroyPipelineCache(logicalDevice, dftPipelineCache, nullptr);
	vkDestroyPipelineLayout(logicalDevice, dftPipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(logicalDevice, dftDescriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(logicalDevice, dftDescriptorPool, nullptr);
	delete fontTexture;
	dftBuffer->unmap();
	delete dftBuffer;

	vkDestroyPipeline(logicalDevice, clPipeline, nullptr);
	vkDestroyPipelineCache(logicalDevice, clPipelineCache, nullptr);
	vkDestroyPipelineLayout(logicalDevice, clPipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(logicalDevice, clDescriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(logicalDevice, clDescriptorPool, nullptr);
	clBuffer->unmap();
	delete clBuffer;

	delete uboBuffer;
}
//------------------------------------------------------------------------------------------------


//Needs-External-Update---------------------------------------------------------------------------
/* Note: */
void Demo::VimguiCore::buildCommandBuffer(VkCommandBuffer cmdBuf)
{
	if (prepared)
	{
		VkDeviceSize offsets[1] = { 0 };
		//ColorQuads------------------------------------------------------------------------------
		vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, cqPipeline);
		vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, cqPipelineLayout, 0, 1, &cqDescriptor, 0, NULL);
		vkCmdBindVertexBuffers(cmdBuf, vertexBufferBindID, 1, &cqBuffer->buffer, offsets);
		if (cqBuffer->vertexCount != 0)
		{
			vkCmdDraw(cmdBuf, cqBuffer->vertexCount, 1, 0, 0);
		}
		//----------------------------------------------------------------------------------------

		//DistanceFieldText-----------------------------------------------------------------------
		vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, dftNormalPipeline);
		vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, dftPipelineLayout, 0, 1, &dftDescriptor, 0, NULL);
		vkCmdBindVertexBuffers(cmdBuf, vertexBufferBindID, 1, &dftBuffer->buffer, offsets);
		if (dftBuffer->vertexCount != 0)
		{
			vkCmdDraw(cmdBuf, dftBuffer->vertexCount, 1, 0, 0);
		}
		//-----------------------------------------------------------------------------------------

		//ColorLine--------------------------------------------------------------------------------
		vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, clPipeline);
		vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, clPipelineLayout, 0, 1, &clDescriptor, 0, NULL);
		vkCmdBindVertexBuffers(cmdBuf, vertexBufferBindID, 1, &clBuffer->buffer, offsets);
		if (clBuffer->vertexCount != 0)
		{
			vkCmdDraw(cmdBuf, clBuffer->vertexCount, 1, 0, 0);
		}
		//-----------------------------------------------------------------------------------------
	}
}

void Demo::VimguiCore::onResize(uint32_t width, uint32_t height)
{
	swapchainExtentWidth = width;
	swapchainExtentHeight = height;
	updateUniformBuffers();
}
//------------------------------------------------------------------------------------------------


//General-and-Helper-Funcs------------------------------------------------------------------------
/*  Note: */
void Demo::VimguiCore::loadPNG(std::vector<unsigned char>& buffer, const char* filename)
{
	std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);

	//get filesize
	std::streamsize size = 0;
	if (file.seekg(0, std::ios::end).good()) size = file.tellg();
	if (file.seekg(0, std::ios::beg).good()) size -= file.tellg();

	//read contents of the file into the vector
	if (size > 0)
	{
		buffer.resize((size_t)size);
		file.read((char*)(&buffer[0]), size);
	}
	else buffer.clear();
}

int Demo::VimguiCore::decodePNG(std::vector<unsigned char>& out_image, unsigned long& image_width, unsigned long& image_height, const unsigned char* in_png, size_t in_size, bool convert_to_rgba32 = true)
{
	// picoPNG version 20101224
	// Copyright (c) 2005-2010 Lode Vandevenne
	//
	// This software is provided 'as-is', without any express or implied
	// warranty. In no event will the authors be held liable for any damages
	// arising from the use of this software.
	//
	// Permission is granted to anyone to use this software for any purpose,
	// including commercial applications, and to alter it and redistribute it
	// freely, subject to the following restrictions:
	//
	//     1. The origin of this software must not be misrepresented; you must not
	//     claim that you wrote the original software. If you use this software
	//     in a product, an acknowledgment in the product documentation would be
	//     appreciated but is not required.
	//     2. Altered source versions must be plainly marked as such, and must not be
	//     misrepresented as being the original software.
	//     3. This notice may not be removed or altered from any source distribution.

	// picoPNG is a PNG decoder in one C++ function of around 500 lines. Use picoPNG for
	// programs that need only 1 .cpp file. Since it's a single function, it's very limited,
	// it can convert a PNG to raw pixel data either converted to 32-bit RGBA color or
	// with no color conversion at all. For anything more complex, another tiny library
	// is available: LodePNG (lodepng.c(pp)), which is a single source and header file.
	// Apologies for the compact code style, it's to make this tiny.

	static const unsigned long LENBASE[29] = { 3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258 };
	static const unsigned long LENEXTRA[29] = { 0,0,0,0,0,0,0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4,  4,  5,  5,  5,  5,  0 };
	static const unsigned long DISTBASE[30] = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577 };
	static const unsigned long DISTEXTRA[30] = { 0,0,0,0,1,1,2, 2, 3, 3, 4, 4, 5, 5,  6,  6,  7,  7,  8,  8,   9,   9,  10,  10,  11,  11,  12,   12,   13,   13 };
	static const unsigned long CLCL[19] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 }; //code length code lengths
	struct Zlib //nested functions for zlib decompression
	{
		static unsigned long readBitFromStream(size_t& bitp, const unsigned char* bits) { unsigned long result = (bits[bitp >> 3] >> (bitp & 0x7)) & 1; bitp++; return result; }
		static unsigned long readBitsFromStream(size_t& bitp, const unsigned char* bits, size_t nbits)
		{
			unsigned long result = 0;
			for (size_t i = 0; i < nbits; i++) result += (readBitFromStream(bitp, bits)) << i;
			return result;
		}
		struct HuffmanTree
		{
			int makeFromLengths(const std::vector<unsigned long>& bitlen, unsigned long maxbitlen)
			{ //make tree given the lengths
				unsigned long numcodes = (unsigned long)(bitlen.size()), treepos = 0, nodefilled = 0;
				std::vector<unsigned long> tree1d(numcodes), blcount(maxbitlen + 1, 0), nextcode(maxbitlen + 1, 0);
				for (unsigned long bits = 0; bits < numcodes; bits++) blcount[bitlen[bits]]++; //count number of instances of each code length
				for (unsigned long bits = 1; bits <= maxbitlen; bits++) nextcode[bits] = (nextcode[bits - 1] + blcount[bits - 1]) << 1;
				for (unsigned long n = 0; n < numcodes; n++) if (bitlen[n] != 0) tree1d[n] = nextcode[bitlen[n]]++; //generate all the codes
				tree2d.clear(); tree2d.resize(numcodes * 2, 32767); //32767 here means the tree2d isn't filled there yet
				for (unsigned long n = 0; n < numcodes; n++) //the codes
					for (unsigned long i = 0; i < bitlen[n]; i++) //the bits for this code
					{
						unsigned long bit = (tree1d[n] >> (bitlen[n] - i - 1)) & 1;
						if (treepos > numcodes - 2) return 55;
						if (tree2d[2 * treepos + bit] == 32767) //not yet filled in
						{
							if (i + 1 == bitlen[n]) { tree2d[2 * treepos + bit] = n; treepos = 0; } //last bit
							else { tree2d[2 * treepos + bit] = ++nodefilled + numcodes; treepos = nodefilled; } //addresses are encoded as values > numcodes
						}
						else treepos = tree2d[2 * treepos + bit] - numcodes; //subtract numcodes from address to get address value
					}
				return 0;
			}
			int decode(bool& decoded, unsigned long& result, size_t& treepos, unsigned long bit) const
			{ //Decodes a symbol from the tree
				unsigned long numcodes = (unsigned long)tree2d.size() / 2;
				if (treepos >= numcodes) return 11; //error: you appeared outside the codetree
				result = tree2d[2 * treepos + bit];
				decoded = (result < numcodes);
				treepos = decoded ? 0 : result - numcodes;
				return 0;
			}
			std::vector<unsigned long> tree2d; //2D representation of a huffman tree: The one dimension is "0" or "1", the other contains all nodes and leaves of the tree.
		};
		struct Inflator
		{
			int error;
			void inflate(std::vector<unsigned char>& out, const std::vector<unsigned char>& in, size_t inpos = 0)
			{
				size_t bp = 0, pos = 0; //bit pointer and byte pointer
				error = 0;
				unsigned long BFINAL = 0;
				while (!BFINAL && !error)
				{
					if (bp >> 3 >= in.size()) { error = 52; return; } //error, bit pointer will jump past memory
					BFINAL = readBitFromStream(bp, &in[inpos]);
					unsigned long BTYPE = readBitFromStream(bp, &in[inpos]); BTYPE += 2 * readBitFromStream(bp, &in[inpos]);
					if (BTYPE == 3) { error = 20; return; } //error: invalid BTYPE
					else if (BTYPE == 0) inflateNoCompression(out, &in[inpos], bp, pos, in.size());
					else inflateHuffmanBlock(out, &in[inpos], bp, pos, in.size(), BTYPE);
				}
				if (!error) out.resize(pos); //Only now we know the true size of out, resize it to that
			}
			void generateFixedTrees(HuffmanTree& tree, HuffmanTree& treeD) //get the tree of a deflated block with fixed tree
			{
				std::vector<unsigned long> bitlen(288, 8), bitlenD(32, 5);;
				for (size_t i = 144; i <= 255; i++) bitlen[i] = 9;
				for (size_t i = 256; i <= 279; i++) bitlen[i] = 7;
				tree.makeFromLengths(bitlen, 15);
				treeD.makeFromLengths(bitlenD, 15);
			}
			HuffmanTree codetree, codetreeD, codelengthcodetree; //the code tree for Huffman codes, dist codes, and code length codes
			unsigned long huffmanDecodeSymbol(const unsigned char* in, size_t& bp, const HuffmanTree& codetree, size_t inlength)
			{ //decode a single symbol from given list of bits with given code tree. return value is the symbol
				bool decoded; unsigned long ct;
				for (size_t treepos = 0;;)
				{
					if ((bp & 0x07) == 0 && (bp >> 3) > inlength) { error = 10; return 0; } //error: end reached without endcode
					error = codetree.decode(decoded, ct, treepos, readBitFromStream(bp, in)); if (error) return 0; //stop, an error happened
					if (decoded) return ct;
				}
			}
			void getTreeInflateDynamic(HuffmanTree& tree, HuffmanTree& treeD, const unsigned char* in, size_t& bp, size_t inlength)
			{ //get the tree of a deflated block with dynamic tree, the tree itself is also Huffman compressed with a known tree
				std::vector<unsigned long> bitlen(288, 0), bitlenD(32, 0);
				if (bp >> 3 >= inlength - 2) { error = 49; return; } //the bit pointer is or will go past the memory
				size_t HLIT = readBitsFromStream(bp, in, 5) + 257; //number of literal/length codes + 257
				size_t HDIST = readBitsFromStream(bp, in, 5) + 1; //number of dist codes + 1
				size_t HCLEN = readBitsFromStream(bp, in, 4) + 4; //number of code length codes + 4
				std::vector<unsigned long> codelengthcode(19); //lengths of tree to decode the lengths of the dynamic tree
				for (size_t i = 0; i < 19; i++) codelengthcode[CLCL[i]] = (i < HCLEN) ? readBitsFromStream(bp, in, 3) : 0;
				error = codelengthcodetree.makeFromLengths(codelengthcode, 7); if (error) return;
				size_t i = 0, replength;
				while (i < HLIT + HDIST)
				{
					unsigned long code = huffmanDecodeSymbol(in, bp, codelengthcodetree, inlength); if (error) return;
					if (code <= 15) { if (i < HLIT) bitlen[i++] = code; else bitlenD[i++ - HLIT] = code; } //a length code
					else if (code == 16) //repeat previous
					{
						if (bp >> 3 >= inlength) { error = 50; return; } //error, bit pointer jumps past memory
						replength = 3 + readBitsFromStream(bp, in, 2);
						unsigned long value; //set value to the previous code
						if ((i - 1) < HLIT) value = bitlen[i - 1];
						else value = bitlenD[i - HLIT - 1];
						for (size_t n = 0; n < replength; n++) //repeat this value in the next lengths
						{
							if (i >= HLIT + HDIST) { error = 13; return; } //error: i is larger than the amount of codes
							if (i < HLIT) bitlen[i++] = value; else bitlenD[i++ - HLIT] = value;
						}
					}
					else if (code == 17) //repeat "0" 3-10 times
					{
						if (bp >> 3 >= inlength) { error = 50; return; } //error, bit pointer jumps past memory
						replength = 3 + readBitsFromStream(bp, in, 3);
						for (size_t n = 0; n < replength; n++) //repeat this value in the next lengths
						{
							if (i >= HLIT + HDIST) { error = 14; return; } //error: i is larger than the amount of codes
							if (i < HLIT) bitlen[i++] = 0; else bitlenD[i++ - HLIT] = 0;
						}
					}
					else if (code == 18) //repeat "0" 11-138 times
					{
						if (bp >> 3 >= inlength) { error = 50; return; } //error, bit pointer jumps past memory
						replength = 11 + readBitsFromStream(bp, in, 7);
						for (size_t n = 0; n < replength; n++) //repeat this value in the next lengths
						{
							if (i >= HLIT + HDIST) { error = 15; return; } //error: i is larger than the amount of codes
							if (i < HLIT) bitlen[i++] = 0; else bitlenD[i++ - HLIT] = 0;
						}
					}
					else { error = 16; return; } //error: somehow an unexisting code appeared. This can never happen.
				}
				if (bitlen[256] == 0) { error = 64; return; } //the length of the end code 256 must be larger than 0
				error = tree.makeFromLengths(bitlen, 15); if (error) return; //now we've finally got HLIT and HDIST, so generate the code trees, and the function is done
				error = treeD.makeFromLengths(bitlenD, 15); if (error) return;
			}
			void inflateHuffmanBlock(std::vector<unsigned char>& out, const unsigned char* in, size_t& bp, size_t& pos, size_t inlength, unsigned long btype)
			{
				if (btype == 1) { generateFixedTrees(codetree, codetreeD); }
				else if (btype == 2) { getTreeInflateDynamic(codetree, codetreeD, in, bp, inlength); if (error) return; }
				for (;;)
				{
					unsigned long code = huffmanDecodeSymbol(in, bp, codetree, inlength); if (error) return;
					if (code == 256) return; //end code
					else if (code <= 255) //literal symbol
					{
						if (pos >= out.size()) out.resize((pos + 1) * 2); //reserve more room
						out[pos++] = (unsigned char)(code);
					}
					else if (code >= 257 && code <= 285) //length code
					{
						size_t length = LENBASE[code - 257], numextrabits = LENEXTRA[code - 257];
						if ((bp >> 3) >= inlength) { error = 51; return; } //error, bit pointer will jump past memory
						length += readBitsFromStream(bp, in, numextrabits);
						unsigned long codeD = huffmanDecodeSymbol(in, bp, codetreeD, inlength); if (error) return;
						if (codeD > 29) { error = 18; return; } //error: invalid dist code (30-31 are never used)
						unsigned long dist = DISTBASE[codeD], numextrabitsD = DISTEXTRA[codeD];
						if ((bp >> 3) >= inlength) { error = 51; return; } //error, bit pointer will jump past memory
						dist += readBitsFromStream(bp, in, numextrabitsD);
						size_t start = pos, back = start - dist; //backwards
						if (pos + length >= out.size()) out.resize((pos + length) * 2); //reserve more room
						for (size_t i = 0; i < length; i++) { out[pos++] = out[back++]; if (back >= start) back = start - dist; }
					}
				}
			}
			void inflateNoCompression(std::vector<unsigned char>& out, const unsigned char* in, size_t& bp, size_t& pos, size_t inlength)
			{
				while ((bp & 0x7) != 0) bp++; //go to first boundary of byte
				size_t p = bp / 8;
				if (p >= inlength - 4) { error = 52; return; } //error, bit pointer will jump past memory
				unsigned long LEN = in[p] + 256 * in[p + 1], NLEN = in[p + 2] + 256 * in[p + 3]; p += 4;
				if (LEN + NLEN != 65535) { error = 21; return; } //error: NLEN is not one's complement of LEN
				if (pos + LEN >= out.size()) out.resize(pos + LEN);
				if (p + LEN > inlength) { error = 23; return; } //error: reading outside of in buffer
				for (unsigned long n = 0; n < LEN; n++) out[pos++] = in[p++]; //read LEN bytes of literal data
				bp = p * 8;
			}
		};
		int decompress(std::vector<unsigned char>& out, const std::vector<unsigned char>& in) //returns error value
		{
			Inflator inflator;
			if (in.size() < 2) { return 53; } //error, size of zlib data too small
			if ((in[0] * 256 + in[1]) % 31 != 0) { return 24; } //error: 256 * in[0] + in[1] must be a multiple of 31, the FCHECK value is supposed to be made that way
			unsigned long CM = in[0] & 15, CINFO = (in[0] >> 4) & 15, FDICT = (in[1] >> 5) & 1;
			if (CM != 8 || CINFO > 7) { return 25; } //error: only compression method 8: inflate with sliding window of 32k is supported by the PNG spec
			if (FDICT != 0) { return 26; } //error: the specification of PNG says about the zlib stream: "The additional flags shall not specify a preset dictionary."
			inflator.inflate(out, in, 2);
			return inflator.error; //note: adler32 checksum was skipped and ignored
		}
	};
	struct PNG //nested functions for PNG decoding
	{
		struct Info
		{
			unsigned long width, height, colorType, bitDepth, compressionMethod, filterMethod, interlaceMethod, key_r, key_g, key_b;
			bool key_defined; //is a transparent color key given?
			std::vector<unsigned char> palette;
		} info;
		int error;
		void decode(std::vector<unsigned char>& out, const unsigned char* in, size_t size, bool convert_to_rgba32)
		{
			error = 0;
			if (size == 0 || in == 0) { error = 48; return; } //the given data is empty
			readPngHeader(&in[0], size); if (error) return;
			size_t pos = 33; //first byte of the first chunk after the header
			std::vector<unsigned char> idat; //the data from idat chunks
			bool IEND = false, known_type = true;
			info.key_defined = false;
			while (!IEND) //loop through the chunks, ignoring unknown chunks and stopping at IEND chunk. IDAT data is put at the start of the in buffer
			{
				if (pos + 8 >= size) { error = 30; return; } //error: size of the in buffer too small to contain next chunk
				size_t chunkLength = read32bitInt(&in[pos]); pos += 4;
				if (chunkLength > 2147483647) { error = 63; return; }
				if (pos + chunkLength >= size) { error = 35; return; } //error: size of the in buffer too small to contain next chunk
				if (in[pos + 0] == 'I' && in[pos + 1] == 'D' && in[pos + 2] == 'A' && in[pos + 3] == 'T') //IDAT chunk, containing compressed image data
				{
					idat.insert(idat.end(), &in[pos + 4], &in[pos + 4 + chunkLength]);
					pos += (4 + chunkLength);
				}
				else if (in[pos + 0] == 'I' && in[pos + 1] == 'E' && in[pos + 2] == 'N' && in[pos + 3] == 'D') { pos += 4; IEND = true; }
				else if (in[pos + 0] == 'P' && in[pos + 1] == 'L' && in[pos + 2] == 'T' && in[pos + 3] == 'E') //palette chunk (PLTE)
				{
					pos += 4; //go after the 4 letters
					info.palette.resize(4 * (chunkLength / 3));
					if (info.palette.size() > (4 * 256)) { error = 38; return; } //error: palette too big
					for (size_t i = 0; i < info.palette.size(); i += 4)
					{
						for (size_t j = 0; j < 3; j++) info.palette[i + j] = in[pos++]; //RGB
						info.palette[i + 3] = 255; //alpha
					}
				}
				else if (in[pos + 0] == 't' && in[pos + 1] == 'R' && in[pos + 2] == 'N' && in[pos + 3] == 'S') //palette transparency chunk (tRNS)
				{
					pos += 4; //go after the 4 letters
					if (info.colorType == 3)
					{
						if (4 * chunkLength > info.palette.size()) { error = 39; return; } //error: more alpha values given than there are palette entries
						for (size_t i = 0; i < chunkLength; i++) info.palette[4 * i + 3] = in[pos++];
					}
					else if (info.colorType == 0)
					{
						if (chunkLength != 2) { error = 40; return; } //error: this chunk must be 2 bytes for greyscale image
						info.key_defined = 1; info.key_r = info.key_g = info.key_b = 256 * in[pos] + in[pos + 1]; pos += 2;
					}
					else if (info.colorType == 2)
					{
						if (chunkLength != 6) { error = 41; return; } //error: this chunk must be 6 bytes for RGB image
						info.key_defined = 1;
						info.key_r = 256 * in[pos] + in[pos + 1]; pos += 2;
						info.key_g = 256 * in[pos] + in[pos + 1]; pos += 2;
						info.key_b = 256 * in[pos] + in[pos + 1]; pos += 2;
					}
					else { error = 42; return; } //error: tRNS chunk not allowed for other color models
				}
				else //it's not an implemented chunk type, so ignore it: skip over the data
				{
					if (!(in[pos + 0] & 32)) { error = 69; return; } //error: unknown critical chunk (5th bit of first byte of chunk type is 0)
					pos += (chunkLength + 4); //skip 4 letters and uninterpreted data of unimplemented chunk
					known_type = false;
				}
				pos += 4; //step over CRC (which is ignored)
			}
			unsigned long bpp = getBpp(info);
			std::vector<unsigned char> scanlines(((info.width * (info.height * bpp + 7)) / 8) + info.height); //now the out buffer will be filled
			Zlib zlib; //decompress with the Zlib decompressor
			error = zlib.decompress(scanlines, idat); if (error) return; //stop if the zlib decompressor returned an error
			size_t bytewidth = (bpp + 7) / 8, outlength = (info.height * info.width * bpp + 7) / 8;
			out.resize(outlength); //time to fill the out buffer
			unsigned char* out_ = outlength ? &out[0] : 0; //use a regular pointer to the std::vector for faster code if compiled without optimization
			if (info.interlaceMethod == 0) //no interlace, just filter
			{
				size_t linestart = 0, linelength = (info.width * bpp + 7) / 8; //length in bytes of a scanline, excluding the filtertype byte
				if (bpp >= 8) //byte per byte
					for (unsigned long y = 0; y < info.height; y++)
					{
						unsigned long filterType = scanlines[linestart];
						const unsigned char* prevline = (y == 0) ? 0 : &out_[(y - 1) * info.width * bytewidth];
						unFilterScanline(&out_[linestart - y], &scanlines[linestart + 1], prevline, bytewidth, filterType, linelength); if (error) return;
						linestart += (1 + linelength); //go to start of next scanline
					}
				else //less than 8 bits per pixel, so fill it up bit per bit
				{
					std::vector<unsigned char> templine((info.width * bpp + 7) >> 3); //only used if bpp < 8
					for (size_t y = 0, obp = 0; y < info.height; y++)
					{
						unsigned long filterType = scanlines[linestart];
						const unsigned char* prevline = (y == 0) ? 0 : &out_[(y - 1) * info.width * bytewidth];
						unFilterScanline(&templine[0], &scanlines[linestart + 1], prevline, bytewidth, filterType, linelength); if (error) return;
						for (size_t bp = 0; bp < info.width * bpp;) setBitOfReversedStream(obp, out_, readBitFromReversedStream(bp, &templine[0]));
						linestart += (1 + linelength); //go to start of next scanline
					}
				}
			}
			else //interlaceMethod is 1 (Adam7)
			{
				size_t passw[7] = { (info.width + 7) / 8, (info.width + 3) / 8, (info.width + 3) / 4, (info.width + 1) / 4, (info.width + 1) / 2, (info.width + 0) / 2, (info.width + 0) / 1 };
				size_t passh[7] = { (info.height + 7) / 8, (info.height + 7) / 8, (info.height + 3) / 8, (info.height + 3) / 4, (info.height + 1) / 4, (info.height + 1) / 2, (info.height + 0) / 2 };
				size_t passstart[7] = { 0 };
				size_t pattern[28] = { 0,4,0,2,0,1,0,0,0,4,0,2,0,1,8,8,4,4,2,2,1,8,8,8,4,4,2,2 }; //values for the adam7 passes
				for (int i = 0; i < 6; i++) passstart[i + 1] = passstart[i] + passh[i] * ((passw[i] ? 1 : 0) + (passw[i] * bpp + 7) / 8);
				std::vector<unsigned char> scanlineo((info.width * bpp + 7) / 8), scanlinen((info.width * bpp + 7) / 8); //"old" and "new" scanline
				for (int i = 0; i < 7; i++)
					adam7Pass(&out_[0], &scanlinen[0], &scanlineo[0], &scanlines[passstart[i]], info.width, pattern[i], pattern[i + 7], pattern[i + 14], pattern[i + 21], passw[i], passh[i], bpp);
			}
			if (convert_to_rgba32 && (info.colorType != 6 || info.bitDepth != 8)) //conversion needed
			{
				std::vector<unsigned char> data = out;
				error = convert(out, &data[0], info, info.width, info.height);
			}
		}
		void readPngHeader(const unsigned char* in, size_t inlength) //read the information from the header and store it in the Info
		{
			if (inlength < 29) { error = 27; return; } //error: the data length is smaller than the length of the header
			if (in[0] != 137 || in[1] != 80 || in[2] != 78 || in[3] != 71 || in[4] != 13 || in[5] != 10 || in[6] != 26 || in[7] != 10) { error = 28; return; } //no PNG signature
			if (in[12] != 'I' || in[13] != 'H' || in[14] != 'D' || in[15] != 'R') { error = 29; return; } //error: it doesn't start with a IHDR chunk!
			info.width = read32bitInt(&in[16]); info.height = read32bitInt(&in[20]);
			info.bitDepth = in[24]; info.colorType = in[25];
			info.compressionMethod = in[26]; if (in[26] != 0) { error = 32; return; } //error: only compression method 0 is allowed in the specification
			info.filterMethod = in[27]; if (in[27] != 0) { error = 33; return; } //error: only filter method 0 is allowed in the specification
			info.interlaceMethod = in[28]; if (in[28] > 1) { error = 34; return; } //error: only interlace methods 0 and 1 exist in the specification
			error = checkColorValidity(info.colorType, info.bitDepth);
		}
		void unFilterScanline(unsigned char* recon, const unsigned char* scanline, const unsigned char* precon, size_t bytewidth, unsigned long filterType, size_t length)
		{
			switch (filterType)
			{
			case 0: for (size_t i = 0; i < length; i++) recon[i] = scanline[i]; break;
			case 1:
				for (size_t i = 0; i < bytewidth; i++) recon[i] = scanline[i];
				for (size_t i = bytewidth; i < length; i++) recon[i] = scanline[i] + recon[i - bytewidth];
				break;
			case 2:
				if (precon) for (size_t i = 0; i < length; i++) recon[i] = scanline[i] + precon[i];
				else       for (size_t i = 0; i < length; i++) recon[i] = scanline[i];
				break;
			case 3:
				if (precon)
				{
					for (size_t i = 0; i < bytewidth; i++) recon[i] = scanline[i] + precon[i] / 2;
					for (size_t i = bytewidth; i < length; i++) recon[i] = scanline[i] + ((recon[i - bytewidth] + precon[i]) / 2);
				}
				else
				{
					for (size_t i = 0; i < bytewidth; i++) recon[i] = scanline[i];
					for (size_t i = bytewidth; i < length; i++) recon[i] = scanline[i] + recon[i - bytewidth] / 2;
				}
				break;
			case 4:
				if (precon)
				{
					for (size_t i = 0; i < bytewidth; i++) recon[i] = scanline[i] + paethPredictor(0, precon[i], 0);
					for (size_t i = bytewidth; i < length; i++) recon[i] = scanline[i] + paethPredictor(recon[i - bytewidth], precon[i], precon[i - bytewidth]);
				}
				else
				{
					for (size_t i = 0; i < bytewidth; i++) recon[i] = scanline[i];
					for (size_t i = bytewidth; i < length; i++) recon[i] = scanline[i] + paethPredictor(recon[i - bytewidth], 0, 0);
				}
				break;
			default: error = 36; return; //error: unexisting filter type given
			}
		}
		void adam7Pass(unsigned char* out, unsigned char* linen, unsigned char* lineo, const unsigned char* in, unsigned long w, size_t passleft, size_t passtop, size_t spacex, size_t spacey, size_t passw, size_t passh, unsigned long bpp)
		{ //filter and reposition the pixels into the output when the image is Adam7 interlaced. This function can only do it after the full image is already decoded. The out buffer must have the correct allocated memory size already.
			if (passw == 0) return;
			size_t bytewidth = (bpp + 7) / 8, linelength = 1 + ((bpp * passw + 7) / 8);
			for (unsigned long y = 0; y < passh; y++)
			{
				unsigned char filterType = in[y * linelength], * prevline = (y == 0) ? 0 : lineo;
				unFilterScanline(linen, &in[y * linelength + 1], prevline, bytewidth, filterType, (w * bpp + 7) / 8); if (error) return;
				if (bpp >= 8) for (size_t i = 0; i < passw; i++) for (size_t b = 0; b < bytewidth; b++) //b = current byte of this pixel
					out[bytewidth * w * (passtop + spacey * y) + bytewidth * (passleft + spacex * i) + b] = linen[bytewidth * i + b];
				else for (size_t i = 0; i < passw; i++)
				{
					size_t obp = bpp * w * (passtop + spacey * y) + bpp * (passleft + spacex * i), bp = i * bpp;
					for (size_t b = 0; b < bpp; b++) setBitOfReversedStream(obp, out, readBitFromReversedStream(bp, &linen[0]));
				}
				unsigned char* temp = linen; linen = lineo; lineo = temp; //swap the two buffer pointers "line old" and "line new"
			}
		}
		static unsigned long readBitFromReversedStream(size_t& bitp, const unsigned char* bits) { unsigned long result = (bits[bitp >> 3] >> (7 - (bitp & 0x7))) & 1; bitp++; return result; }
		static unsigned long readBitsFromReversedStream(size_t& bitp, const unsigned char* bits, unsigned long nbits)
		{
			unsigned long result = 0;
			for (size_t i = nbits - 1; i < nbits; i--) result += ((readBitFromReversedStream(bitp, bits)) << i);
			return result;
		}
		void setBitOfReversedStream(size_t& bitp, unsigned char* bits, unsigned long bit) { bits[bitp >> 3] |= (bit << (7 - (bitp & 0x7))); bitp++; }
		unsigned long read32bitInt(const unsigned char* buffer) { return (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3]; }
		int checkColorValidity(unsigned long colorType, unsigned long bd) //return type is a LodePNG error code
		{
			if ((colorType == 2 || colorType == 4 || colorType == 6)) { if (!(bd == 8 || bd == 16)) return 37; else return 0; }
			else if (colorType == 0) { if (!(bd == 1 || bd == 2 || bd == 4 || bd == 8 || bd == 16)) return 37; else return 0; }
			else if (colorType == 3) { if (!(bd == 1 || bd == 2 || bd == 4 || bd == 8)) return 37; else return 0; }
			else return 31; //unexisting color type
		}
		unsigned long getBpp(const Info& info)
		{
			if (info.colorType == 2) return (3 * info.bitDepth);
			else if (info.colorType >= 4) return (info.colorType - 2) * info.bitDepth;
			else return info.bitDepth;
		}
		int convert(std::vector<unsigned char>& out, const unsigned char* in, Info& infoIn, unsigned long w, unsigned long h)
		{ //converts from any color type to 32-bit. return value = LodePNG error code
			size_t numpixels = w * h, bp = 0;
			out.resize(numpixels * 4);
			unsigned char* out_ = out.empty() ? 0 : &out[0]; //faster if compiled without optimization
			if (infoIn.bitDepth == 8 && infoIn.colorType == 0) //greyscale
				for (size_t i = 0; i < numpixels; i++)
				{
					out_[4 * i + 0] = out_[4 * i + 1] = out_[4 * i + 2] = in[i];
					out_[4 * i + 3] = (infoIn.key_defined && in[i] == infoIn.key_r) ? 0 : 255;
				}
			else if (infoIn.bitDepth == 8 && infoIn.colorType == 2) //RGB color
				for (size_t i = 0; i < numpixels; i++)
				{
					for (size_t c = 0; c < 3; c++) out_[4 * i + c] = in[3 * i + c];
					out_[4 * i + 3] = (infoIn.key_defined == 1 && in[3 * i + 0] == infoIn.key_r && in[3 * i + 1] == infoIn.key_g && in[3 * i + 2] == infoIn.key_b) ? 0 : 255;
				}
			else if (infoIn.bitDepth == 8 && infoIn.colorType == 3) //indexed color (palette)
				for (size_t i = 0; i < numpixels; i++)
				{
					if (4U * in[i] >= infoIn.palette.size()) return 46;
					for (size_t c = 0; c < 4; c++) out_[4 * i + c] = infoIn.palette[4 * in[i] + c]; //get rgb colors from the palette
				}
			else if (infoIn.bitDepth == 8 && infoIn.colorType == 4) //greyscale with alpha
				for (size_t i = 0; i < numpixels; i++)
				{
					out_[4 * i + 0] = out_[4 * i + 1] = out_[4 * i + 2] = in[2 * i + 0];
					out_[4 * i + 3] = in[2 * i + 1];
				}
			else if (infoIn.bitDepth == 8 && infoIn.colorType == 6) for (size_t i = 0; i < numpixels; i++) for (size_t c = 0; c < 4; c++) out_[4 * i + c] = in[4 * i + c]; //RGB with alpha
			else if (infoIn.bitDepth == 16 && infoIn.colorType == 0) //greyscale
				for (size_t i = 0; i < numpixels; i++)
				{
					out_[4 * i + 0] = out_[4 * i + 1] = out_[4 * i + 2] = in[2 * i];
					out_[4 * i + 3] = (infoIn.key_defined && 256U * in[i] + in[i + 1] == infoIn.key_r) ? 0 : 255;
				}
			else if (infoIn.bitDepth == 16 && infoIn.colorType == 2) //RGB color
				for (size_t i = 0; i < numpixels; i++)
				{
					for (size_t c = 0; c < 3; c++) out_[4 * i + c] = in[6 * i + 2 * c];
					out_[4 * i + 3] = (infoIn.key_defined && 256U * in[6 * i + 0] + in[6 * i + 1] == infoIn.key_r && 256U * in[6 * i + 2] + in[6 * i + 3] == infoIn.key_g && 256U * in[6 * i + 4] + in[6 * i + 5] == infoIn.key_b) ? 0 : 255;
				}
			else if (infoIn.bitDepth == 16 && infoIn.colorType == 4) //greyscale with alpha
				for (size_t i = 0; i < numpixels; i++)
				{
					out_[4 * i + 0] = out_[4 * i + 1] = out_[4 * i + 2] = in[4 * i]; //most significant byte
					out_[4 * i + 3] = in[4 * i + 2];
				}
			else if (infoIn.bitDepth == 16 && infoIn.colorType == 6) for (size_t i = 0; i < numpixels; i++) for (size_t c = 0; c < 4; c++) out_[4 * i + c] = in[8 * i + 2 * c]; //RGB with alpha
			else if (infoIn.bitDepth < 8 && infoIn.colorType == 0) //greyscale
				for (size_t i = 0; i < numpixels; i++)
				{
					unsigned long value = (readBitsFromReversedStream(bp, in, infoIn.bitDepth) * 255) / ((1 << infoIn.bitDepth) - 1); //scale value from 0 to 255
					out_[4 * i + 0] = out_[4 * i + 1] = out_[4 * i + 2] = (unsigned char)(value);
					out_[4 * i + 3] = (infoIn.key_defined && value && ((1U << infoIn.bitDepth) - 1U) == infoIn.key_r && ((1U << infoIn.bitDepth) - 1U)) ? 0 : 255;
				}
			else if (infoIn.bitDepth < 8 && infoIn.colorType == 3) //palette
				for (size_t i = 0; i < numpixels; i++)
				{
					unsigned long value = readBitsFromReversedStream(bp, in, infoIn.bitDepth);
					if (4 * value >= infoIn.palette.size()) return 47;
					for (size_t c = 0; c < 4; c++) out_[4 * i + c] = infoIn.palette[4 * value + c]; //get rgb colors from the palette
				}
			return 0;
		}
		unsigned char paethPredictor(short a, short b, short c) //Paeth predicter, used by PNG filter type 4
		{
			short p = a + b - c, pa = p > a ? (p - a) : (a - p), pb = p > b ? (p - b) : (b - p), pc = p > c ? (p - c) : (c - p);
			return (unsigned char)((pa <= pb && pa <= pc) ? a : pb <= pc ? b : c);
		}
	};
	PNG decoder; decoder.decode(out_image, in_png, in_size, convert_to_rgba32);
	image_width = decoder.info.width; image_height = decoder.info.height;
	return decoder.error;
}

Demo::UITexture* Demo::VimguiCore::loadImage(const char* filepath)
{
	UITexture* UItexture;
	UItexture = new UITexture(logicalDevice);
	UItexture->filepath = filepath;

	//load and decode
	std::vector<unsigned char> buffer, image;
	loadPNG(buffer, UItexture->filepath);
	unsigned long w, h;
	int error = decodePNG(image, w, h, buffer.empty() ? 0 : &buffer[0], (unsigned long)buffer.size());
	//if there's an error, display it
	if (error != 0)
	{
		std::cout << "error: " << error << std::endl;
	}
	//the pixels are now in the vector "image", use it as texture, draw it, ...
	if (image.size() > 4)
	{
		std::cout << "width: " << w << " height: " << h << " first pixel: " << std::hex << int(image[0]) << int(image[1]) << int(image[2]) << int(image[3]) << std::endl;
	}
	UItexture->textureWidth = w;
	UItexture->textureHeight = h;
	UItexture->textureImageMemorySize = UItexture->textureWidth * UItexture->textureHeight * 4;
	Demo::UIBuffer* stagingBuffer;
	stagingBuffer = new UIBuffer(logicalDevice, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, UItexture->textureImageMemorySize);
	createBuffer(stagingBuffer, image.data() );
	//stbi_image_free(image);

	createImage(UItexture->textureWidth, UItexture->textureHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, UItexture->textureImage, UItexture->textureImageMemory);
	transitionImageLayout(UItexture->textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copyBufferToImage(stagingBuffer->buffer, UItexture->textureImage, static_cast<uint32_t>(UItexture->textureWidth), static_cast<uint32_t>(UItexture->textureHeight));
	transitionImageLayout(UItexture->textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	stagingBuffer->destroy();
	delete stagingBuffer;
	stagingBuffer = 0;
	setupTexture(UItexture);

	return UItexture;
}

void Demo::VimguiCore::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width = width;
	imageCreateInfo.extent.height = height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.format = format;
	imageCreateInfo.tiling = tiling;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.usage = usage;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult result;
	result = vkCreateImage(logicalDevice, &imageCreateInfo, nullptr, &image);

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(logicalDevice, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(logicalDevice, image, imageMemory, 0);
}

void Demo::VimguiCore::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	//Creating pool------------------------------------------------------------------
	VkResult result;
	VkCommandPool commandPool;
	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = graphicsQueueIndex;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	result = vkCreateCommandPool(logicalDevice, &commandPoolCreateInfo, nullptr, &commandPool);
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanDevice: Failed to Create Local CommandPool" << std::endl;
	}
	//------------------------------------------------------------------------------

	//Creating buffer---------------------------------------------------------------
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	result = vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer);
	if (result != VK_SUCCESS)
	{
		std::cout << "Vimgui: Failed to allocate commandbuffer!" << std::endl;
	}
	//----------------------------------------------------------------------------------

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;
	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		throw std::invalid_argument("unsupported layout transition!");
	}


	vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	//Destroy pool
	//destroy buffer
	vkEndCommandBuffer(commandBuffer);
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	// Create fence to ensure that the command buffer has finished executing
	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = 0;
	VkFence fence;
	vkCreateFence(logicalDevice, &fenceCreateInfo, nullptr, &fence);
	// Submit to the queue
	vkQueueSubmit(graphicsQueue, 1, &submitInfo, fence);
	// Wait for the fence to signal that command buffer has finished executing
	vkWaitForFences(logicalDevice, 1, &fence, VK_TRUE, 100000000000);
	vkDestroyFence(logicalDevice, fence, nullptr);
	vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);

	vkDestroyCommandPool(logicalDevice, commandPool, nullptr);

}

void Demo::VimguiCore::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	//Create pool
	//Alloc buffer

	//Creating pool------------------------------------------------------------------
	VkResult result;
	VkCommandPool commandPool;
	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = graphicsQueueIndex;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	result = vkCreateCommandPool(logicalDevice, &commandPoolCreateInfo, nullptr, &commandPool);
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanDevice: Failed to Create Local CommandPool" << std::endl;
	}
	//------------------------------------------------------------------------------

	//Creating buffer---------------------------------------------------------------
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	result = vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer);
	if (result != VK_SUCCESS)
	{
		std::cout << "Vimgui: Failed to allocate commandbuffer!" << std::endl;
	}
	//----------------------------------------------------------------------------------

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { width, height, 1 };
	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	vkEndCommandBuffer(commandBuffer);
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	// Create fence to ensure that the command buffer has finished executing
	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = 0;
	VkFence fence;
	vkCreateFence(logicalDevice, &fenceCreateInfo, nullptr, &fence);
	// Submit to the queue
	vkQueueSubmit(graphicsQueue, 1, &submitInfo, fence);
	// Wait for the fence to signal that command buffer has finished executing
	vkWaitForFences(logicalDevice, 1, &fence, VK_TRUE, 100000000000);
	vkDestroyFence(logicalDevice, fence, nullptr);
	vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);

	vkDestroyCommandPool(logicalDevice, commandPool, nullptr);

}

void Demo::VimguiCore::setupTexture(UITexture* texture)
{
	//TextureImageView------------------------------------------------------------------------
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = texture->textureImage;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;
	VkResult result;
	result = vkCreateImageView(logicalDevice, &viewInfo, nullptr, &texture->textureImageView);
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanTexture: Failed to create image view for texture!" << std::endl;
	}
	//----------------------------------------------------------------------------------------

	//TextureSampler--------------------------------------------------------------------------
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;
	result = vkCreateSampler(logicalDevice, &samplerInfo, nullptr, &texture->textureSampler);
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanTexture: Failed to create image sampler for texture!" << std::endl;
	}
	//----------------------------------------------------------------------------------------

	//TextureDescriptor-----------------------------------------------------------------------
	texture->textureDescriptor.imageLayout = texture->textureImageLayout;
	texture->textureDescriptor.imageView = texture->textureImageView;
	texture->textureDescriptor.sampler = texture->textureSampler;
	//----------------------------------------------------------------------------------------
}

uint32_t Demo::VimguiCore::findMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound)
{
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);
	for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((typeBits & 1) == 1)
		{
			if ((physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				if (memTypeFound)
				{
					*memTypeFound = true;
				}
				return i;
			}
		}
		typeBits >>= 1;
	}

	if (memTypeFound)
	{
		*memTypeFound = false;
		return 0;
	}
	else
	{
		std::cout << "VulkanDevice: Could not find memory type!" << std::endl;
	}
}

void Demo::VimguiCore::createBuffer(UIBuffer* buffer, void* data = nullptr)
{
	buffer->device = logicalDevice;
	VkResult result;

	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.usage = buffer->usageFlags;
	bufferCreateInfo.size = buffer->size;
	result = vkCreateBuffer(buffer->device, &bufferCreateInfo, nullptr, &buffer->buffer);
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanDevice: Failed to create buffer!" << std::endl;
	}

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(logicalDevice, buffer->buffer, &memoryRequirements);
	buffer->alignment = memoryRequirements.alignment;

	VkMemoryAllocateInfo memoryAllocateInfo{};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, buffer->memoryPropertyFlags);
	result = vkAllocateMemory(buffer->device, &memoryAllocateInfo, nullptr, &buffer->memory);
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanDevice: Failed to allocate buffer memory!" << std::endl;
	}

	if (data != nullptr)
	{
		VkResult result;
		result = buffer->map();
		if (result != VK_SUCCESS)
		{
			std::cout << "VulkanDevice: Failed to map memory in buffer" << std::endl;
		}
		memcpy(buffer->mapped, data, buffer->size);
		if ((buffer->memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
		{
			std::cout << "VulkanDevice: Flushed" << std::endl;
			buffer->flush();
		}

		buffer->unmap();
	}
	buffer->setupDescriptor();
	result = buffer->bind();
	if (result != VK_SUCCESS)
	{
		std::cout << "VulkanDevice: Failed to bind buffer memory!" << std::endl;
	}
}

void Demo::VimguiCore::prepUniformBuffers()
{
	uboBuffer = new UIBuffer(logicalDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(ubo));
	createBuffer(uboBuffer);
	updateUniformBuffers();
}

void Demo::VimguiCore::updateUniformBuffers()
{
	ubo.projectionMatrix = glm::ortho(0.0f, static_cast<float>(swapchainExtentWidth), 0.0f, static_cast<float>(swapchainExtentHeight), 1.0f, -1.0f);
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	ubo.modelMatrix = glm::mat4(1.0f);
	ubo.modelMatrix = glm::translate(ubo.modelMatrix, glm::vec3(position));

	VkResult result;
	result = uboBuffer->map();
	if (result != VK_SUCCESS)
	{
		std::cout << "DistanceFieldFont: Failed to map UBO buffer memory!" << std::endl;
	}
	memcpy(uboBuffer->mapped, &ubo, sizeof(ubo));
	uboBuffer->unmap();
}

VkShaderModule Demo::VimguiCore::loadShader(const char* filepath)
{
	std::ifstream file(filepath, std::ios::ate | std::ios::binary);
	if (!file.is_open()) { throw std::runtime_error("failed to open Shader file!"); VkShaderModule noShaderLoaded; return noShaderLoaded; }
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> shaderCode(fileSize);
	file.seekg(0);
	file.read(shaderCode.data(), fileSize);
	file.close();

	VkShaderModule shaderModule;
	VkShaderModuleCreateInfo shaderCreateInfo = {};
	shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderCreateInfo.codeSize = shaderCode.size();
	shaderCreateInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

	if (vkCreateShaderModule(logicalDevice, &shaderCreateInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create Shader module!");
	}
	return shaderModule;
}
//------------------------------------------------------------------------------------------------


//Color-Quad-Public-Funcs-------------------------------------------------------------------------
/* Note: */
uint32_t Demo::VimguiCore::addColorQuad(ColorQuad element)
{
	if (cqElementID == 0)
	{
		cqElementID++;
	}
	if (element.elementID == 0)
	{
		element.elementID = cqElementID;
	}
	addColorQuadElement(element);
	cqElementID++;
	float elementID = static_cast<float>(element.elementID);
	float scale = element.scale;
	float width = element.size.x * scale;
	float height = element.size.y * scale;
	float posx = element.position.x;
	float posy = element.position.y;
	float posz = element.position.z;
	float clrr = Math::Map(element.color.r, 0.0f, 255.0f, 0.0f, 1.0f);
	float clrg = Math::Map(element.color.g, 0.0f, 255.0f, 0.0f, 1.0f);
	float clrb = Math::Map(element.color.b, 0.0f, 255.0f, 0.0f, 1.0f);
	float clra = Math::Map(element.color.a, 0.0f, 255.0f, 0.0f, 1.0f);
	cqCurrentBufferSize = cqVertices.size();
	if (cqCurrentBufferSize < cqMaxElements)
	{
		//CLOCKWISE WINDING, ORIGIN TOP LEFT. IE P1.TOP LEFT = x, y, P4.BOTTOM RIGHT = x + width, y + height.  0,1,2, 2,3,0
		//0.TOPLEFT------vert:1
		float id = elementID + cqIDMatch;
		cqVertices.push_back(posx);
		cqVertices.push_back(posy);
		cqVertices.push_back(posz);
		cqVertices.push_back(id);
		cqVertices.push_back(clrr);
		cqVertices.push_back(clrg);
		cqVertices.push_back(clrb);
		cqVertices.push_back(clra);
		//1.TOPRIGHT--------vert:2
		cqVertices.push_back(posx + width);
		cqVertices.push_back(posy);
		cqVertices.push_back(posz);
		cqVertices.push_back(id);
		cqVertices.push_back(clrr);
		cqVertices.push_back(clrg);
		cqVertices.push_back(clrb);
		cqVertices.push_back(clra);
		//2.BOTRIGHT--------vert:3
		cqVertices.push_back(posx + width);
		cqVertices.push_back(posy + height);
		cqVertices.push_back(posz);
		cqVertices.push_back(id);
		cqVertices.push_back(clrr);
		cqVertices.push_back(clrg);
		cqVertices.push_back(clrb);
		cqVertices.push_back(clra);
		//Next triangle
		//2.BOTRIGHT--------vert:4
		cqVertices.push_back(posx + width);
		cqVertices.push_back(posy + height);
		cqVertices.push_back(posz);
		cqVertices.push_back(id);
		cqVertices.push_back(clrr);
		cqVertices.push_back(clrg);
		cqVertices.push_back(clrb);
		cqVertices.push_back(clra);
		//3.BOTLEFT--------vert:5
		cqVertices.push_back(posx);
		cqVertices.push_back(posy + height);
		cqVertices.push_back(posz);
		cqVertices.push_back(id);
		cqVertices.push_back(clrr);
		cqVertices.push_back(clrg);
		cqVertices.push_back(clrb);
		cqVertices.push_back(clra);
		//0.TOPLEFT------vert:6
		cqVertices.push_back(posx);
		cqVertices.push_back(posy);
		cqVertices.push_back(posz);
		cqVertices.push_back(id);
		cqVertices.push_back(clrr);
		cqVertices.push_back(clrg);
		cqVertices.push_back(clrb);
		cqVertices.push_back(clra);

	}
	cqBuffer->vertexCount = cqVertices.size() / 8;

	cqCurrentBufferSize = cqVertices.size();
	if (cqCurrentBufferSize > cqMaxElements) 
	{
		std::cout << "VimguiCore: Error! ColorQuadBuffer size limit reached!" << std::endl;
		prepared = false;
		return 0;
	}
	else
	{
		updateColorQuads();
		return element.elementID;
	}
}

float Demo::VimguiCore::findColorQuad(uint32_t posx, uint32_t posy)
{
	float id = 0;
	for (uint32_t i = 0; i < cqElementBuffer.size(); ++i)
	{
		if (posx >= cqElementBuffer[i].position.x - 20.0f && posx <= cqElementBuffer[i].position.x + cqElementBuffer[i].size.x)
		{
			if (posy >= cqElementBuffer[i].position.y - 20.0f && posy <= cqElementBuffer[i].position.y + cqElementBuffer[i].size.y)
			{
				id = i;
				return id;
			}
		}
	}
	return -1.0f;

}

void Demo::VimguiCore::positionColorQuad(uint32_t id, uint32_t posx, uint32_t posy)
{
	/* Note: This functions sets the position of an element contained in colorQuad vertex buffer. */

	/* Edit: I feel like this is an utter terrible way of changing the vertex buffer, but its the only way i can do it, without rebuilding the buffer.
		Rebuildig means clearing the entire vertices buffer, then going through each element in element buffer, and pushing them back to vert buffer.*/
	uint32_t elementBufferIndexOfElement;
	float width, height; //quad dimensions
	for (uint32_t i = 0; i < cqElementBuffer.size(); ++i)
	{
		if (id == cqElementBuffer[i].elementID)
		{
			//Element ID found!
			//Find dimensions of element.
			//Store index for later usage.
			elementBufferIndexOfElement = i;
			width = cqElementBuffer[i].size.x;
			height = cqElementBuffer[i].size.y;
			cqElementBuffer[i].position.x = posx;
			cqElementBuffer[i].position.y = posy;
		}
	}

	float match = id + cqIDMatch;
	for (uint32_t i = 0; i < cqVertices.size(); ++i)
	{
		if (cqVertices[i] == match)
		{
			//ID match in vertex buffer.
			//Attempt to change all affected elements.
			//Current Vert layout;
			//posx 1
			//posy 2
			//posz 3
			//id   4
			//clrr 5
			//clrg 6
			//clrb 7
			//clra 8
			//... 48
			
			uint32_t indexposxv1 = i - 3;
			uint32_t indexposyv1 = i - 2;

			uint32_t indexposxv2 = i + 8 - 3;
			uint32_t indexposyv2 = i + 8 - 2;

			uint32_t indexposxv3 = i + 16 - 3;
			uint32_t indexposyv3 = i + 16 - 2;

			uint32_t indexposxv4 = i + 24 - 3;
			uint32_t indexposyv4 = i + 24 - 2;

			uint32_t indexposxv5 = i + 32 - 3;
			uint32_t indexposyv5 = i + 32 - 2;

			uint32_t indexposxv6 = i + 40 - 3;
			uint32_t indexposyv6 = i + 40 - 2;

			cqVertices[indexposxv1] = posx;
			cqVertices[indexposyv1] = posy;

			cqVertices[indexposxv2] = posx + width;
			cqVertices[indexposyv2] = posy;

			cqVertices[indexposxv3] = posx + width;
			cqVertices[indexposyv3] = posy + height;

			cqVertices[indexposxv4] = posx + width;
			cqVertices[indexposyv4] = posy + height;

			cqVertices[indexposxv5] = posx;
			cqVertices[indexposyv5] = posy + height;

			cqVertices[indexposxv6] = posx;
			cqVertices[indexposyv6] = posy;
			break; //Break out of function since we're changing all the elements at once, instead of find first match, find second, third etc..
		}
	}

	updateColorQuads();
}

void Demo::VimguiCore::colorEditColorQuad(uint32_t id, float r, float g, float b, float a)
{
	/* Note: This functions sets the position of an element contained in colorQuad vertex buffer. */

	/* Edit: I feel like this is an utter terrible way of changing the vertex buffer, but its the only way i can do it, without rebuilding the buffer.
	Rebuildig means clearing the entire vertices buffer, then going through each element in element buffer, and pushing them back to vert buffer.*/
	uint32_t elementBufferIndexOfElement;
	for (uint32_t i = 0; i < cqElementBuffer.size(); ++i)
	{
		if (id == cqElementBuffer[i].elementID)
		{
			//Element ID found!
			//Find dimensions of element.
			//Store index for later usage.
			elementBufferIndexOfElement = i;
			cqElementBuffer[i].color.r = Math::Map(r, 0.0f, 255.0f, 0.0f, 1.0f);
			cqElementBuffer[i].color.g = Math::Map(g, 0.0f, 255.0f, 0.0f, 1.0f);
			cqElementBuffer[i].color.b = Math::Map(b, 0.0f, 255.0f, 0.0f, 1.0f);
			cqElementBuffer[i].color.a = Math::Map(a, 0.0f, 255.0f, 0.0f, 1.0f);
		}
	}

	float match = id + cqIDMatch;
	for (uint32_t i = 0; i < cqVertices.size(); ++i)
	{
		if (cqVertices[i] == match)
		{
			//ID match in vertex buffer.
			//Current Vert layout;
			//posx 1
			//posy 2
			//posz 3
			//id   4
			//clrr 5
			//clrg 6
			//clrb 7
			//clra 8
			uint32_t indexclrrv1 = i + 1;
			uint32_t indexclrgv1 = i + 2;
			uint32_t indexclrbv1 = i + 3;
			uint32_t indexclrav1 = i + 4;
			cqVertices[indexclrrv1] = Math::Map(r, 0.0f, 255.0f, 0.0f, 1.0f);
			cqVertices[indexclrgv1] = Math::Map(g, 0.0f, 255.0f, 0.0f, 1.0f);
			cqVertices[indexclrbv1] = Math::Map(b, 0.0f, 255.0f, 0.0f, 1.0f);
			cqVertices[indexclrav1] = Math::Map(a, 0.0f, 255.0f, 0.0f, 1.0f);
			
		}
	}

	updateColorQuads();
}

void Demo::VimguiCore::removeColorQuad(uint32_t id)
{
	float match = id + cqIDMatch;
	
	for (uint32_t i = 0; i < cqVertices.size(); ++i)
	{
		if (cqVertices[i] == match)
		{
			//cqVert
			//posx	1 
			//posy	2
			//posz	3
			//ID	4
			//clrR	5
			//clrG	6
			//clrB	7
			//clrA	8
			uint32_t begin;
			begin = i - 3;
			uint32_t end;
			end = begin + 8;
			//check that we're not seeking outside bounds
			if (begin <= cqVertices.size() && begin >= 0 && end <= cqVertices.size() && end >= 0)
			{
				cqVertices.erase(cqVertices.begin() + begin, cqVertices.begin() + end);
				i = 0;
			}
			else
			{
				std::cout << "VimguiCore: Error resizing colorQuadBuffer, consider rebuilding!" << std::endl;
			}
		}
	}
	cqBuffer->vertexCount = cqVertices.size() / 8;
	updateColorQuads();
}

void Demo::VimguiCore::removeColorQuad(uint32_t posx, uint32_t posy)
{
	for (uint32_t i = 0; i < cqElementBuffer.size(); ++i)
	{
		if (posx >= cqElementBuffer[i].position.x && posx <= cqElementBuffer[i].position.x + cqElementBuffer[i].size.x)
		{
			if (posy >= cqElementBuffer[i].position.y && posy <= cqElementBuffer[i].position.y + cqElementBuffer[i].size.y)
			{
				//std::cout << "element removed. id: " << cqElementBuffer[i].elementID << std::endl;
				removeColorQuad(cqElementBuffer[i].elementID);
				removeColorQuadElement(cqElementBuffer[i].elementID);
				break;
			}
		}

	}
}

void Demo::VimguiCore::removeColorQuadElement(uint32_t id)
{
	for (uint32_t i = 0; i < cqElementBuffer.size(); ++i)
	{
		if (id == cqElementBuffer[i].elementID)
		{
			uint32_t begin;
			begin = i;
			//check that we're not seeking outside bounds
			if (begin <= cqElementBuffer.size() && begin >= 0)
			{
				cqElementBuffer.erase(cqElementBuffer.begin() + begin);
			}
		}
	}
}

void Demo::VimguiCore::addColorQuadElement(ColorQuad cq)
{
	//TODO, add basic error checking, regarding size and whatnot
	cqElementBuffer.push_back(cq);
	//std::cout << "element added. id: " << cq.elementID << std::endl;
}

void Demo::VimguiCore::rebuildColorQuadBuffer()
{
	prepared = false;
	cqVertices.clear();
	cqVertices.shrink_to_fit();

	for (uint32_t i = 0; i < cqElementBuffer.size(); ++i)
	{
		float elementID = static_cast<float>(cqElementBuffer[i].elementID);
		float scale  = cqElementBuffer[i].scale;
		float width  = cqElementBuffer[i].size.x * scale;
		float height = cqElementBuffer[i].size.y * scale;
		float posx   = cqElementBuffer[i].position.x;
		float posy   = cqElementBuffer[i].position.y;
		float posz   = cqElementBuffer[i].position.z;
		float clrr   = Math::Map(cqElementBuffer[i].color.r, 0.0f, 255.0f, 0.0f, 1.0f);
		float clrg   = Math::Map(cqElementBuffer[i].color.g, 0.0f, 255.0f, 0.0f, 1.0f);
		float clrb   = Math::Map(cqElementBuffer[i].color.b, 0.0f, 255.0f, 0.0f, 1.0f);
		float clra   = Math::Map(cqElementBuffer[i].color.a, 0.0f, 255.0f, 0.0f, 1.0f);
		float id = elementID + cqIDMatch;

		cqVertices.push_back(posx);
		cqVertices.push_back(posy);
		cqVertices.push_back(posz);
		cqVertices.push_back(id);
		cqVertices.push_back(clrr);
		cqVertices.push_back(clrg);
		cqVertices.push_back(clrb);
		cqVertices.push_back(clra);
		//1.TOPRIGHT--------vert:2
		cqVertices.push_back(posx + width);
		cqVertices.push_back(posy);
		cqVertices.push_back(posz);
		cqVertices.push_back(id);
		cqVertices.push_back(clrr);
		cqVertices.push_back(clrg);
		cqVertices.push_back(clrb);
		cqVertices.push_back(clra);
		//2.BOTRIGHT--------vert:3
		cqVertices.push_back(posx + width);
		cqVertices.push_back(posy + height);
		cqVertices.push_back(posz);
		cqVertices.push_back(id);
		cqVertices.push_back(clrr);
		cqVertices.push_back(clrg);
		cqVertices.push_back(clrb);
		cqVertices.push_back(clra);
		//Next triangle
		//2.BOTRIGHT--------vert:4
		cqVertices.push_back(posx + width);
		cqVertices.push_back(posy + height);
		cqVertices.push_back(posz);
		cqVertices.push_back(id);
		cqVertices.push_back(clrr);
		cqVertices.push_back(clrg);
		cqVertices.push_back(clrb);
		cqVertices.push_back(clra);
		//3.BOTLEFT--------vert:5
		cqVertices.push_back(posx);
		cqVertices.push_back(posy + height);
		cqVertices.push_back(posz);
		cqVertices.push_back(id);
		cqVertices.push_back(clrr);
		cqVertices.push_back(clrg);
		cqVertices.push_back(clrb);
		cqVertices.push_back(clra);
		//0.TOPLEFT------vert:6
		cqVertices.push_back(posx);
		cqVertices.push_back(posy);
		cqVertices.push_back(posz);
		cqVertices.push_back(id);
		cqVertices.push_back(clrr);
		cqVertices.push_back(clrg);
		cqVertices.push_back(clrb);
		cqVertices.push_back(clra);
	}
	prepared = true;
	cqBuffer->vertexCount = cqVertices.size() / 8;
	updateColorQuads();
}

void Demo::VimguiCore::updateColorQuads()
{
	//TODO; add error checking for src + payload size not exceeding dst size
	size_t size = cqVertices.size() * sizeof(float);
	memcpy(cqBuffer->mapped, cqVertices.data(), size);
}
//------------------------------------------------------------------------------------------------


//Distance-Field-Text-Public-Funcs----------------------------------------------------------------
/* Note: */
uint32_t Demo::VimguiCore::addDistanceFieldText(DistanceFieldText element)
{
	if (dftElementID == 0)
	{
		dftElementID++;
	}
	if (element.elementID == 0)
	{
		element.elementID = dftElementID;
	}
	addDistanceFieldTextElement(element);
	dftElementID++;
	std::string::const_iterator c;
	float elementID = static_cast<float>(element.elementID);
	elementID = elementID + dftIDMatch;
	float w = fontTexture->textureWidth;
	float posx = element.position.x;
	float posy = element.position.y;
	float posz = element.position.z;
	float scale = element.scale;
	float colorr = element.color.r;
	float colorg = element.color.g;
	float colorb = element.color.b;
	float colora = element.color.a;

	dftCurrentBufferSize = dftVertices.size();
	if (dftCurrentBufferSize < dftMaxElements)
	{
		for (uint32_t i = 0; i < element.text.size(); ++i)
		{
			fontChar* charInfo = &fontChars[(uint32_t)element.text[i]];
			if (charInfo->width == 0) { charInfo->width = 36; }

			float charw = charInfo->width;
			float charh = charInfo->height;
			float dimx = scale * charw;
			float dimy = scale * charh;
			float xo = scale * charInfo->xoffset;
			float yo = scale * charInfo->yoffset;
			float bl = charInfo->x / w;
			float ul = charInfo->y / w;
			float br = (charInfo->x + charInfo->width) / w;
			float ur = (charInfo->y + charInfo->height) / w;

			//0-X,Y-------------------------------------
			dftVertices.push_back(posx + xo);
			dftVertices.push_back(posy + yo);
			dftVertices.push_back(posz);
			dftVertices.push_back(elementID);
			//U,V
			dftVertices.push_back(bl);
			dftVertices.push_back(ul);
			//colorR,cG,cB,cA
			dftVertices.push_back(colorr);
			dftVertices.push_back(colorg);
			dftVertices.push_back(colorb);
			dftVertices.push_back(colora);
			//-------------------------------------------

			//1-X,Y-------------------------------------
			dftVertices.push_back(posx + dimx + xo);
			dftVertices.push_back(posy + yo);
			dftVertices.push_back(posz);
			dftVertices.push_back(elementID);
			//U,V
			dftVertices.push_back(br);
			dftVertices.push_back(ul);
			//colorR,cG,cB,cA
			dftVertices.push_back(colorr);
			dftVertices.push_back(colorg);
			dftVertices.push_back(colorb);
			dftVertices.push_back(colora);
			//-------------------------------------------

			//2-X,Y-------------------------------------
			dftVertices.push_back(posx + dimx + xo);
			dftVertices.push_back(posy + dimy + yo);
			dftVertices.push_back(posz);
			dftVertices.push_back(elementID);
			//U,V
			dftVertices.push_back(br);
			dftVertices.push_back(ur);
			//colorR,cG,cB,cA
			dftVertices.push_back(colorr);
			dftVertices.push_back(colorg);
			dftVertices.push_back(colorb);
			dftVertices.push_back(colora);
			//-------------------------------------------

			//2-X,Y-------------------------------------
			dftVertices.push_back(posx + dimx + xo);
			dftVertices.push_back(posy + dimy + yo);
			dftVertices.push_back(posz);
			dftVertices.push_back(elementID);
			//U,V
			dftVertices.push_back(br);
			dftVertices.push_back(ur);
			//colorR,cG,cB,cA
			dftVertices.push_back(colorr);
			dftVertices.push_back(colorg);
			dftVertices.push_back(colorb);
			dftVertices.push_back(colora);
			//-------------------------------------------

			//3-X,Y-------------------------------------
			dftVertices.push_back(posx + xo);
			dftVertices.push_back(posy + dimy + yo);
			dftVertices.push_back(posz);
			dftVertices.push_back(elementID);
			//U,V
			dftVertices.push_back(bl);
			dftVertices.push_back(ur);
			//colorR,cG,cB,cA
			dftVertices.push_back(colorr);
			dftVertices.push_back(colorg);
			dftVertices.push_back(colorb);
			dftVertices.push_back(colora);
			//-------------------------------------------

			//0-X,Y-------------------------------------
			dftVertices.push_back(posx + xo);
			dftVertices.push_back(posy + yo);
			dftVertices.push_back(posz);
			dftVertices.push_back(elementID);
			//U,V
			dftVertices.push_back(bl);
			dftVertices.push_back(ul);
			//colorR,cG,cB,cA
			dftVertices.push_back(colorr);
			dftVertices.push_back(colorg);
			dftVertices.push_back(colorb);
			dftVertices.push_back(colora);
			//-------------------------------------------
			float advance = scale * (float)charInfo->xadvance * 1.2;
			posx += advance;
		}
	}

	dftBuffer->vertexCount = dftVertices.size() / 10;

	dftCurrentBufferSize = dftVertices.size();
	if (dftCurrentBufferSize > dftMaxElements)
	{
		std::cout << "VimguiCore: Error! DistanceFieldTextBuffer size limit reached!" << std::endl;
		prepared = false;
		return 0;
	}
	else
	{
		updateDistanceFieldText();
		return element.elementID;
	}

}

float Demo::VimguiCore::findDistanceFieldText(uint32_t posx, uint32_t posy)
{
	float id = 0;
	for (uint32_t i = 0; i < dftElementBuffer.size(); ++i)
	{

		if (posx >= dftElementBuffer[i].position.x - 20.0f && posx <= dftElementBuffer[i].position.x + 20.0f)
		{

			if (posy >= dftElementBuffer[i].position.y - 20.0f && posy <= dftElementBuffer[i].position.y + 20.0f)
			{
				id = i;
				return id;
			}
		}
	}
	return -1.0f;
}

void Demo::VimguiCore::positionDistanceFieldText(uint32_t id, uint32_t posx, uint32_t posy)
{

	/* Note: Since each quaf for dft buffer is different, we just remove the old element, and a copy of it with a new pos. */
	DistanceFieldText newElement;
	for (uint32_t i = 0; i < dftElementBuffer.size(); ++i)
	{
		if (id == dftElementBuffer[i].elementID)
		{
			//Element ID found!
			//Find dimensions of element.
			//Store index for later usage.
			newElement.color = dftElementBuffer[i].color;
			newElement.scale = dftElementBuffer[i].scale;
			newElement.text =	dftElementBuffer[i].text;
			newElement.position.x = posx;
			newElement.position.y = posy;
			newElement.position.z = dftElementBuffer[i].position.z;
			newElement.elementID = dftElementBuffer[i].elementID; //If id is already set, then the ID counter of the element will NOT increment.
			break;
		}
	}
	removeDistanceFieldText(id);
	removeDistanceFieldTextElement(id);
	addDistanceFieldText(newElement);
	updateColorQuads();
}

void Demo::VimguiCore::colorEditDistanceFieldText(uint32_t id, float r, float g, float b, float a)
{
	uint32_t elementBufferIndexOfElement;
	for (uint32_t i = 0; i < dftElementBuffer.size(); ++i)
	{
		if (id == dftElementBuffer[i].elementID)
		{
			//Element ID found!
			//Find dimensions of element.
			//Store index for later usage.
			elementBufferIndexOfElement = i;
			dftElementBuffer[i].color.r = Math::Map(r, 0.0f, 255.0f, 0.0f, 1.0f);
			dftElementBuffer[i].color.g = Math::Map(g, 0.0f, 255.0f, 0.0f, 1.0f);
			dftElementBuffer[i].color.b = Math::Map(b, 0.0f, 255.0f, 0.0f, 1.0f);
			dftElementBuffer[i].color.a = Math::Map(a, 0.0f, 255.0f, 0.0f, 1.0f);
		}
	}

	float match = id + dftIDMatch;
	for (uint32_t i = 0; i < dftVertices.size(); ++i)
	{
		if (dftVertices[i] == match)
		{
			//ID found in vertex buffer.
			//Layout is currentyl;
			//posx
			//posy
			//posz
			//id
			//tex x
			//tex y
			//clr r
			//clr g
			//clr b
			//clr a
			uint32_t indexclrrv1 = i + 3;
			uint32_t indexclrgv1 = i + 4;
			uint32_t indexclrbv1 = i + 5;
			uint32_t indexclrav1 = i + 6;
			dftVertices[indexclrrv1] = Math::Map(r, 0.0f, 255.0f, 0.0f, 1.0f);
			dftVertices[indexclrgv1] = Math::Map(g, 0.0f, 255.0f, 0.0f, 1.0f);
			dftVertices[indexclrbv1] = Math::Map(b, 0.0f, 255.0f, 0.0f, 1.0f);
			dftVertices[indexclrav1] = Math::Map(a, 0.0f, 255.0f, 0.0f, 1.0f);
			
		}
	}
	updateDistanceFieldText();
}

void Demo::VimguiCore::removeDistanceFieldText(uint32_t id)
{
	float match = id + dftIDMatch;
	for (uint32_t i = 0; i < dftVertices.size(); ++i)
	{
		if (dftVertices[i] == match)
		{
			//dftVert
			//posx	1 
			//posy	2
			//posz	3
			//ID	4
			//uvx	5
			//uby	6
			//clrR	7
			//clrG	8
			//clrB	9
			//clrA	10
			uint32_t begin;
			begin = i - 3;
			uint32_t end;
			end = begin + 10;
			//check that we're not seeking outside bounds
			if (begin <= dftVertices.size() && begin >= 0 && end <= dftVertices.size() && end >= 0)
			{
				dftVertices.erase(dftVertices.begin() + begin, dftVertices.begin() + end);
				i = 0;
			}
			else
			{
				std::cout << "VimguiCore: Error resizing DistanceFieldTextBuffer, consider rebuilding!" << std::endl;
			}
		}
	}
	dftBuffer->vertexCount = dftVertices.size() / 10;
	updateDistanceFieldText();
}

void Demo::VimguiCore::removeDistanceFieldText(uint32_t posx, uint32_t posy)
{
	for (uint32_t i = 0; i < dftElementBuffer.size(); ++i)
	{
		if (posx >= dftElementBuffer[i].position.x && posx <= dftElementBuffer[i].position.x + 20)
		{
			if (posy >= dftElementBuffer[i].position.y && posy <= dftElementBuffer[i].position.y + 20)
			{
				//std::cout << "element removed. id: " << dftElementBuffer[i].elementID << std::endl;
				removeDistanceFieldText(dftElementBuffer[i].elementID);
				removeDistanceFieldTextElement(dftElementBuffer[i].elementID);
				break;
			}
		}

	}
}

void Demo::VimguiCore::removeDistanceFieldTextElement(uint32_t id)
{
	for (uint32_t i = 0; i < dftElementBuffer.size(); ++i)
	{
		if (id == dftElementBuffer[i].elementID)
		{
			uint32_t begin;
			begin = i;
			//check that we're not seeking outside bounds
			if (begin <= dftElementBuffer.size() && begin >= 0)
			{
				dftElementBuffer.erase(dftElementBuffer.begin() + begin);
			}
		}
	}
}

void Demo::VimguiCore::addDistanceFieldTextElement(DistanceFieldText dft)
{
	//TODO, add basic error checking, regarding size and whatnot
	dftElementBuffer.push_back(dft);
	//std::cout << "element added. id: " << dft.elementID << std::endl;
}

void Demo::VimguiCore::rebuildDistanceFieldTextBuffer()
{
	prepared = false;
	dftVertices.clear();
	dftVertices.shrink_to_fit();

	for (uint32_t i = 0; i < dftElementBuffer.size(); ++i)
	{

		glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
		std::string::const_iterator c;
		float elementID = static_cast<float>(dftElementBuffer[i].elementID);
		elementID = elementID + dftIDMatch;
		float w = fontTexture->textureWidth;
		float posx = dftElementBuffer[i].position.x;
		float posy = dftElementBuffer[i].position.y;
		float posz = dftElementBuffer[i].position.z;
		float scale = dftElementBuffer[i].scale;
		float colorr = dftElementBuffer[i].color.r;
		float colorg = dftElementBuffer[i].color.g;
		float colorb = dftElementBuffer[i].color.b;
		float colora = dftElementBuffer[i].color.a;

		dftCurrentBufferSize = dftVertices.size();
		if (dftCurrentBufferSize < dftMaxElements)
		{
			for (uint32_t j = 0; j < dftElementBuffer[i].text.size(); ++j)
			{
				fontChar* charInfo = &fontChars[(uint32_t)dftElementBuffer[i].text[j]];
				if (charInfo->width == 0) { charInfo->width = 36; }

				float charw = charInfo->width;
				float charh = charInfo->height;
				float dimx = scale * charw;
				float dimy = scale * charh;
				float xo = scale * charInfo->xoffset;
				float yo = scale * charInfo->yoffset;
				float bl = charInfo->x / w;
				float ul = charInfo->y / w;
				float br = (charInfo->x + charInfo->width) / w;
				float ur = (charInfo->y + charInfo->height) / w;

				//0-X,Y-------------------------------------
				dftVertices.push_back(posx + xo);
				dftVertices.push_back(posy + yo);
				dftVertices.push_back(posz);
				dftVertices.push_back(elementID);
				//U,V
				dftVertices.push_back(bl);
				dftVertices.push_back(ul);
				//colorR,cG,cB,cA
				dftVertices.push_back(colorr);
				dftVertices.push_back(colorg);
				dftVertices.push_back(colorb);
				dftVertices.push_back(colora);
				//-------------------------------------------

				//1-X,Y-------------------------------------
				dftVertices.push_back(posx + dimx + xo);
				dftVertices.push_back(posy + yo);
				dftVertices.push_back(posz);
				dftVertices.push_back(elementID);
				//U,V
				dftVertices.push_back(br);
				dftVertices.push_back(ul);
				//colorR,cG,cB,cA
				dftVertices.push_back(colorr);
				dftVertices.push_back(colorg);
				dftVertices.push_back(colorb);
				dftVertices.push_back(colora);
				//-------------------------------------------

				//2-X,Y-------------------------------------
				dftVertices.push_back(posx + dimx + xo);
				dftVertices.push_back(posy + dimy + yo);
				dftVertices.push_back(posz);
				dftVertices.push_back(elementID);
				//U,V
				dftVertices.push_back(br);
				dftVertices.push_back(ur);
				//colorR,cG,cB,cA
				dftVertices.push_back(colorr);
				dftVertices.push_back(colorg);
				dftVertices.push_back(colorb);
				dftVertices.push_back(colora);
				//-------------------------------------------

				//2-X,Y-------------------------------------
				dftVertices.push_back(posx + dimx + xo);
				dftVertices.push_back(posy + dimy + yo);
				dftVertices.push_back(posz);
				dftVertices.push_back(elementID);
				//U,V
				dftVertices.push_back(br);
				dftVertices.push_back(ur);
				//colorR,cG,cB,cA
				dftVertices.push_back(colorr);
				dftVertices.push_back(colorg);
				dftVertices.push_back(colorb);
				dftVertices.push_back(colora);
				//-------------------------------------------

				//3-X,Y-------------------------------------
				dftVertices.push_back(posx + xo);
				dftVertices.push_back(posy + dimy + yo);
				dftVertices.push_back(posz);
				dftVertices.push_back(elementID);
				//U,V
				dftVertices.push_back(bl);
				dftVertices.push_back(ur);
				//colorR,cG,cB,cA
				dftVertices.push_back(colorr);
				dftVertices.push_back(colorg);
				dftVertices.push_back(colorb);
				dftVertices.push_back(colora);
				//-------------------------------------------

				//0-X,Y-------------------------------------
				dftVertices.push_back(posx + xo);
				dftVertices.push_back(posy + yo);
				dftVertices.push_back(posz);
				dftVertices.push_back(elementID);
				//U,V
				dftVertices.push_back(bl);
				dftVertices.push_back(ul);
				//colorR,cG,cB,cA
				dftVertices.push_back(colorr);
				dftVertices.push_back(colorg);
				dftVertices.push_back(colorb);
				dftVertices.push_back(colora);
				//-------------------------------------------
				float advance = scale * (float)charInfo->xadvance * 1.2;
				posx += advance;
			}
		}
		dftBuffer->vertexCount = dftVertices.size() / 10;
		prepared = true;
		updateDistanceFieldText();
	}
}

void Demo::VimguiCore::updateDistanceFieldText()
{
	size_t size = dftVertices.size() * sizeof(float);
	memcpy(dftBuffer->mapped, dftVertices.data(), size);
}
//------------------------------------------------------------------------------------------------


//Color-Line-Public-Funcs-------------------------------------------------------------------------
/* Note: */
uint32_t Demo::VimguiCore::addColorLine(ColorLine element)
{
	if (clElementID == 0)
	{
		clElementID++;
	}
	if (element.elementID == 0) //Its uninitialized
	{
		element.elementID = clElementID; //Set ID to ID Counter
	}
	addColorLineElement(element);
	clElementID++;
	float elementID = static_cast<float>(element.elementID);
	elementID = elementID + clIDMatch;

	float p1clrr = Math::Map(element.p1Color.r, 0.0f, 255.0f, 0.0f, 1.0f);
	float p1clrg = Math::Map(element.p1Color.g, 0.0f, 255.0f, 0.0f, 1.0f);
	float p1clrb = Math::Map(element.p1Color.b, 0.0f, 255.0f, 0.0f, 1.0f);
	float p1clra = Math::Map(element.p1Color.a, 0.0f, 255.0f, 0.0f, 1.0f);

	float p2clrr = Math::Map(element.p2Color.r, 0.0f, 255.0f, 0.0f, 1.0f);
	float p2clrg = Math::Map(element.p2Color.g, 0.0f, 255.0f, 0.0f, 1.0f);
	float p2clrb = Math::Map(element.p2Color.b, 0.0f, 255.0f, 0.0f, 1.0f);
	float p2clra = Math::Map(element.p2Color.a, 0.0f, 255.0f, 0.0f, 1.0f);

	clCurrentBufferSize = clVertices.size();
	if (clCurrentBufferSize < clMaxElements)
	{
		clVertices.push_back(element.p2Pos.x);
		clVertices.push_back(element.p2Pos.y);
		clVertices.push_back(element.p2Pos.z);
		clVertices.push_back(elementID);
		clVertices.push_back(p2clrr);
		clVertices.push_back(p2clrg);
		clVertices.push_back(p2clrb);
		clVertices.push_back(p2clra);

		clVertices.push_back(element.p1Pos.x);
		clVertices.push_back(element.p1Pos.y);
		clVertices.push_back(element.p1Pos.z);
		clVertices.push_back(elementID);
		clVertices.push_back(p1clrr);
		clVertices.push_back(p1clrg);
		clVertices.push_back(p1clrb);
		clVertices.push_back(p1clra);
	}

	clBuffer->vertexCount = clVertices.size() / 8;

	clCurrentBufferSize = clVertices.size();
	if (clCurrentBufferSize > clMaxElements)
	{
		std::cout << "VimguiCore: Error! ColorLineBuffer size limit reached!" << std::endl;
		prepared = false;
		return 0;
	}
	else
	{
		updateColorLines();
		return element.elementID;
	}
}

float Demo::VimguiCore::findColorLine(uint32_t posx, uint32_t posy)
{
	float id = 0;
	for (uint32_t i = 0; i < clElementBuffer.size(); ++i)
	{
		if (posx >= clElementBuffer[i].p1Pos.x - 20.0f && posx <= clElementBuffer[i].p2Pos.x + 20.0f)
		{
			if (posy >= clElementBuffer[i].p1Pos.y - 20.0f && posy <= clElementBuffer[i].p2Pos.y + 20.0f)
			{
				id = i;
				return id;
			}
		}
		else if (posx >= clElementBuffer[i].p2Pos.x - 20.0f && posx <= clElementBuffer[i].p1Pos.x + 20.0f)
		{
			if (posy >= clElementBuffer[i].p2Pos.y - 20.0f && posy <= clElementBuffer[i].p1Pos.y + 20.0f)
			{
				id = i;
				return id;
			}
		}
	}
	return -1.0f;
}

void Demo::VimguiCore::positionColorLine(uint32_t id, uint32_t posx, uint32_t posy)
{

	/* Note: Since each quaf for dft buffer is different, we just remove the old element, and add a copy of it with a new pos. */
	ColorLine newElement;
	for (uint32_t i = 0; i < clElementBuffer.size(); ++i)
	{
		if (id == clElementBuffer[i].elementID)
		{
			//Element ID found!
			//Find dimensions of element.
			//Store index for later usage.
			newElement.p1Color = clElementBuffer[i].p1Color;
			newElement.p2Color = clElementBuffer[i].p2Color;
			newElement.p1Pos = clElementBuffer[i].p1Pos;
			newElement.p2Pos = clElementBuffer[i].p2Pos;
			newElement.elementID = clElementBuffer[i].elementID; //If id is already set, then the ID counter of the element will NOT increment.
			break;
		}
	}
	removeColorLine(id);
	removeColorLineElement(id);
	addColorLine(newElement);
	updateColorLines();
}

void Demo::VimguiCore::colorEditColorLine(uint32_t id, float r, float g, float b, float a)
{
	uint32_t elementBufferIndexOfElement;
	for (uint32_t i = 0; i < clElementBuffer.size(); ++i)
	{
		if (id == clElementBuffer[i].elementID)
		{
			//Element ID found!
			//Find dimensions of element.
			//Store index for later usage.
			elementBufferIndexOfElement = i;
			clElementBuffer[i].p1Color.r = Math::Map(r, 0.0f, 255.0f, 0.0f, 1.0f);
			clElementBuffer[i].p1Color.g = Math::Map(g, 0.0f, 255.0f, 0.0f, 1.0f);
			clElementBuffer[i].p1Color.b = Math::Map(b, 0.0f, 255.0f, 0.0f, 1.0f);
			clElementBuffer[i].p1Color.a = Math::Map(a, 0.0f, 255.0f, 0.0f, 1.0f);

			clElementBuffer[i].p2Color.r = Math::Map(r, 0.0f, 255.0f, 0.0f, 1.0f);
			clElementBuffer[i].p2Color.g = Math::Map(g, 0.0f, 255.0f, 0.0f, 1.0f);
			clElementBuffer[i].p2Color.b = Math::Map(b, 0.0f, 255.0f, 0.0f, 1.0f);
			clElementBuffer[i].p2Color.a = Math::Map(a, 0.0f, 255.0f, 0.0f, 1.0f);
		}
	}

	float match = id + clIDMatch;
	for (uint32_t i = 0; i < clVertices.size(); ++i)
	{
		if (clVertices[i] == match)
		{
			//ID match in vertex buffer.
			//Current Vert layout;
			//posx
			//posy
			//posz
			//id
			//clr r
			//clr g
			//clr b
			//clr a
			uint32_t indexclrrv1 = i + 1;
			uint32_t indexclrgv1 = i + 2;
			uint32_t indexclrbv1 = i + 3;
			uint32_t indexclrav1 = i + 4;
			clVertices[indexclrrv1] = Math::Map(r, 0.0f, 255.0f, 0.0f, 1.0f);
			clVertices[indexclrgv1] = Math::Map(g, 0.0f, 255.0f, 0.0f, 1.0f);
			clVertices[indexclrbv1] = Math::Map(b, 0.0f, 255.0f, 0.0f, 1.0f);
			clVertices[indexclrav1] = Math::Map(a, 0.0f, 255.0f, 0.0f, 1.0f);

		}
	}

	updateColorLines();
}

void Demo::VimguiCore::removeColorLine(uint32_t id)
{
	float match = id + clIDMatch;
	for (uint32_t i = 0; i < clVertices.size(); ++i)
	{
		if (clVertices[i] == match)
		{
			//cqVert
			//posx	1 
			//posy	2
			//posz	3
			//ID	4
			//clrR	5
			//clrG	6
			//clrB	7
			//clrA	8
			uint32_t begin;
			begin = i - 3;
			uint32_t end;
			end = begin + 8;
			//check that we're not seeking outside bounds
			if (begin <= clVertices.size() && begin >= 0 && end <= clVertices.size() && end >= 0)
			{
				clVertices.erase(clVertices.begin() + begin, clVertices.begin() + end);
				i = 0;
			}
			else
			{
				std::cout << "VimguiCore: Error resizing colorQuadBuffer, consider rebuilding!" << std::endl;
			}
		}
	}
	clBuffer->vertexCount = clVertices.size() / 8;
	updateColorLines();;
}

void Demo::VimguiCore::removeColorLine(uint32_t posx, uint32_t posy)
{
	for (uint32_t i = 0; i < clElementBuffer.size(); ++i)
	{
		//This will rarely work, we'll have to create a box surrounding the line, then checking for hits within that box
		//or maybe add a range, x is fair enough, but y should have some p1PosY-5, p2posY+5
		if (posx >= clElementBuffer[i].p1Pos.x && posx <= clElementBuffer[i].p2Pos.x)
		{

			if (posy >= clElementBuffer[i].p1Pos.y - 5.0f && posy <= clElementBuffer[i].p2Pos.y + 5.0f)
			{
				//std::cout << "element removed. id: " << cqElementBuffer[i].elementID << std::endl;
				removeColorLine(clElementBuffer[i].elementID);
				removeColorLineElement(clElementBuffer[i].elementID);
				break;
			}
		}

	}
	clBuffer->vertexCount = clVertices.size() / 8;
	updateColorLines();
}

void Demo::VimguiCore::removeColorLineElement(uint32_t id)
{
	for (uint32_t i = 0; i < clElementBuffer.size(); ++i)
	{
		if (id == clElementBuffer[i].elementID)
		{
			uint32_t begin;
			begin = i;
			//check that we're not seeking outside bounds
			if (begin <= clElementBuffer.size() && begin >= 0)
			{
				clElementBuffer.erase(clElementBuffer.begin() + begin);
			}
		}
	}
}

void Demo::VimguiCore::addColorLineElement(ColorLine cl)
{
	//TODO, add basic error checking, regarding size and whatnot
	clElementBuffer.push_back(cl);
	//std::cout << "element added. id: " << cl.elementID << std::endl;
}

void Demo::VimguiCore::rebuildColorLineBuffer()
{
	prepared = false;
	clVertices.clear();
	clVertices.shrink_to_fit();


	for (uint32_t i = 0; i < clElementBuffer.size(); ++i)
	{
		clVertices.push_back(clElementBuffer[i].p2Pos.x);
		clVertices.push_back(clElementBuffer[i].p2Pos.y);
		clVertices.push_back(clElementBuffer[i].p2Pos.z);
		clVertices.push_back(clElementBuffer[i].elementID);
		clVertices.push_back(clElementBuffer[i].p2Color.r);
		clVertices.push_back(clElementBuffer[i].p2Color.g);
		clVertices.push_back(clElementBuffer[i].p2Color.b);
		clVertices.push_back(clElementBuffer[i].p2Color.a);

		clVertices.push_back(clElementBuffer[i].p1Pos.x);
		clVertices.push_back(clElementBuffer[i].p1Pos.y);
		clVertices.push_back(clElementBuffer[i].p1Pos.z);
		clVertices.push_back(clElementBuffer[i].elementID);
		clVertices.push_back(clElementBuffer[i].p1Color.r);
		clVertices.push_back(clElementBuffer[i].p1Color.g);
		clVertices.push_back(clElementBuffer[i].p1Color.b);
		clVertices.push_back(clElementBuffer[i].p1Color.a);
	}

	prepared = true;
	clBuffer->vertexCount = clVertices.size() / 8;
	updateColorLines();
}

void Demo::VimguiCore::updateColorLines()
{
	//TODO; add error checking for src + payload size not exceeding dst size
	size_t size = clVertices.size() * sizeof(float);
	memcpy(clBuffer->mapped, clVertices.data(), size);
}
//------------------------------------------------------------------------------------------------


//Color-Quad-Private-Funcs------------------------------------------------------------------------
/* Note: */
void Demo::VimguiCore::cqPrepPipelineCache()
{
	//ColorQuad-------------------------------------------------------------------------------------------------
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	VkResult result;
	result = vkCreatePipelineCache(logicalDevice, &pipelineCacheCreateInfo, nullptr, &cqPipelineCache);
	if (result != VK_SUCCESS)
	{
		std::cout << "VimguiCore_ColorQuad: Failed to create pipeline cache!" << std::endl;
	}
	//----------------------------------------------------------------------------------------------------------
}

void Demo::VimguiCore::cqPrepVertexInputDescription()
{
	//ColorQuad-------------------------------------------------------------------------------------------------
	VkVertexInputBindingDescription vertexInputBindingDescription = {};
	vertexInputBindingDescription.binding = vertexBufferBindID;
	vertexInputBindingDescription.stride = sizeof(float) * 8;
	vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	cpBindingDescriptions.push_back(vertexInputBindingDescription);

	VkVertexInputAttributeDescription vertexInputAttributeDescription = {};
	vertexInputAttributeDescription.binding = vertexBufferBindID;
	vertexInputAttributeDescription.location = 0;
	vertexInputAttributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	vertexInputAttributeDescription.offset = 0;
	cpAttributeDescriptions.push_back(vertexInputAttributeDescription);

	vertexInputAttributeDescription.location = 1;
	vertexInputAttributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	vertexInputAttributeDescription.offset = sizeof(float) * 4;
	cpAttributeDescriptions.push_back(vertexInputAttributeDescription);
	//----------------------------------------------------------------------------------------------------------
}

void Demo::VimguiCore::cqPrepDescriptorSetLayout()
{
	//ColorQuad-------------------------------------------------------------------------------------------------
	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBinding = {};
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.pImmutableSamplers = nullptr;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.binding = 0;											//Descriptor Binding
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;	//Descriptor Type
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;				//Shader Stage Flag
	descriptorSetLayoutBinding.push_back(uboLayoutBinding);
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.bindingCount = descriptorSetLayoutBinding.size();
	descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBinding.data();
	VkResult result;
	result = vkCreateDescriptorSetLayout(logicalDevice, &descriptorSetLayoutCreateInfo, nullptr, &cqDescriptorSetLayout);
	//----------------------------------------------------------------------------------------------------------
}

void Demo::VimguiCore::cqPrepPipelineLayout()
{
	//ColorQuad-------------------------------------------------------------------------------------------------
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &cqDescriptorSetLayout;
	vkCreatePipelineLayout(logicalDevice, &pipelineLayoutCreateInfo, nullptr, &cqPipelineLayout);
	//----------------------------------------------------------------------------------------------------------
}

void Demo::VimguiCore::cqPrepPipeline()
{
	VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{};
	pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipelineInputAssemblyStateCreateInfo.flags = 0;
	pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo{};
	pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	pipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
	pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;
	pipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	pipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
	pipelineRasterizationStateCreateInfo.flags = 0;
	pipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	pipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;

	VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState{};
	pipelineColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	pipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	pipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	pipelineColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	pipelineColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	pipelineColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	pipelineColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
	pipelineColorBlendAttachmentState.blendEnable = VK_TRUE;

	VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
	pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	pipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
	pipelineColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
	pipelineColorBlendStateCreateInfo.blendConstants[0] = 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[1] = 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[2] = 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[3] = 0.0f;
	pipelineColorBlendStateCreateInfo.pAttachments = &pipelineColorBlendAttachmentState;
	pipelineColorBlendStateCreateInfo.attachmentCount = 1;

	VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{};
	pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	pipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
	pipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
	pipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
	pipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{};
	pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	pipelineViewportStateCreateInfo.viewportCount = 1;
	pipelineViewportStateCreateInfo.scissorCount = 1;

	VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{};
	pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	pipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	pipelineMultisampleStateCreateInfo.minSampleShading = 1.0f;
	pipelineMultisampleStateCreateInfo.pSampleMask = nullptr;
	pipelineMultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
	pipelineMultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

	std::vector<VkDynamicState> dynamicStateEnabled = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
	pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	pipelineDynamicStateCreateInfo.dynamicStateCount = dynamicStateEnabled.size();
	pipelineDynamicStateCreateInfo.pDynamicStates = dynamicStateEnabled.data();

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	VkShaderModule vertexShaderModule = loadShader("shaders\\2D\\colorpanel\\colorpanel1.vert.spv");
	VkShaderModule fragmentShaderModule = loadShader("shaders\\2D\\colorpanel\\colorpanel1.frag.spv");
	VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo{};
	pipelineShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pipelineShaderStageCreateInfo.pName = "main"; //Shader Entrypoint
	pipelineShaderStageCreateInfo.module = vertexShaderModule;
	pipelineShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages.push_back(pipelineShaderStageCreateInfo);

	pipelineShaderStageCreateInfo.module = fragmentShaderModule;
	pipelineShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages.push_back(pipelineShaderStageCreateInfo);

	VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
	pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(cpBindingDescriptions.size());
	pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(cpAttributeDescriptions.size());
	pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = cpBindingDescriptions.data();
	pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = cpAttributeDescriptions.data();

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
	graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCreateInfo.layout = cqPipelineLayout;
	graphicsPipelineCreateInfo.renderPass = renderPass;
	graphicsPipelineCreateInfo.flags = 0;
	graphicsPipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
	graphicsPipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
	graphicsPipelineCreateInfo.pViewportState = &pipelineViewportStateCreateInfo;
	graphicsPipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;
	graphicsPipelineCreateInfo.pDynamicState = &pipelineDynamicStateCreateInfo;
	graphicsPipelineCreateInfo.stageCount = shaderStages.size();
	graphicsPipelineCreateInfo.pStages = shaderStages.data();
	graphicsPipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;

	VkResult result;
	result = vkCreateGraphicsPipelines(logicalDevice, cqPipelineCache, 1, &graphicsPipelineCreateInfo, nullptr, &cqPipeline);
	if (result != VK_SUCCESS)
	{
		std::cout << "DistanceFieldFont: Error creating normal pipeline!" << std::endl;
	}

	vkDestroyShaderModule(logicalDevice, vertexShaderModule, nullptr);
	vkDestroyShaderModule(logicalDevice, fragmentShaderModule, nullptr);
}

void Demo::VimguiCore::cqPrepDescriptorPool()
{
	//ColorQuad-------------------------------------------------------------------------------------------------
	std::vector<VkDescriptorPoolSize> poolSizes = {};

	VkDescriptorPoolSize descriptorPoolSize{};
	descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorPoolSize.descriptorCount = 1;
	poolSizes.push_back(descriptorPoolSize);

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();
	descriptorPoolCreateInfo.poolSizeCount = poolSizes.size();
	descriptorPoolCreateInfo.maxSets = 1;

	VkResult result;
	result = vkCreateDescriptorPool(logicalDevice, &descriptorPoolCreateInfo, nullptr, &cqDescriptorPool);
	if (result != VK_SUCCESS)
	{
		std::cout << "DistanceFieldFont: Error creating descriptor pool" << std::endl;
	}
	//----------------------------------------------------------------------------------------------------------
}

void Demo::VimguiCore::cqPrepDescriptorSet()
{
	//ColorQuad-------------------------------------------------------------------------------------------------
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = cqDescriptorPool;
	descriptorSetAllocateInfo.pSetLayouts = &cqDescriptorSetLayout;
	descriptorSetAllocateInfo.descriptorSetCount = 1;

	VkResult result;
	result = vkAllocateDescriptorSets(logicalDevice, &descriptorSetAllocateInfo, &cqDescriptor);

	std::vector<VkWriteDescriptorSet> writeDescriptorSets = {};
	VkWriteDescriptorSet writeDescriptorSet{};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.dstSet = cqDescriptor;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSet.dstBinding = 0;
	writeDescriptorSet.pBufferInfo = &uboBuffer->descriptor;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSets.push_back(writeDescriptorSet);

	vkUpdateDescriptorSets(logicalDevice, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);
	//----------------------------------------------------------------------------------------------------------
}

void Demo::VimguiCore::cqPrepColorQuadBuffer()
{
	//Create a size vector that represents our buffers maximum size
	std::vector<float> sizeVector;
	//Resize vector so buffer can hold "maxElements"
	sizeVector.resize(cqMaxElements); //Each vertex is represented by 7 floats, each element is represented by 6 vertices.
	//Calculate buffer size
	size_t vertexBufferSize = sizeVector.size();
	//Generate buffer
	cqBuffer = new UIBuffer(logicalDevice, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBufferSize);
	createBuffer(cqBuffer);
	//Map buffer memory, storing pointer to mapped memory in buffer container.
	cqBuffer->map();
	//Cleaning up size vector
	sizeVector.resize(0);
	sizeVector.clear();
	sizeVector.shrink_to_fit();
	/*	
	*	Buffer is now prepared, and can be written to as easy as; 
	*	size_t size = colorQuadVertices.size() * sizeof(float);
	*	memcpy(colorQuadBuffer->mapped, colorQuadVertices.data(), size); 
	*/
}
//------------------------------------------------------------------------------------------------


//Distance-Field-Text-Private-Funcs---------------------------------------------------------------
/* Note: */
void Demo::VimguiCore::parseFontFile(const char* filepath)
{
	std::stringstream fnt;
	fnt << filepath << ".fnt";
	std::filebuf fileBuffer;
	fileBuffer.open(fnt.str().c_str(), std::ios::in);
	std::istream fileStream(&fileBuffer);

	assert(fileStream.good());

	//While we are not at the end of file
	while (!fileStream.eof())
	{
		//Get line from filestream
		std::string line;
		std::getline(fileStream, line);
		//Push line into stringstream
		std::stringstream lineStream;
		lineStream << line;
		//Push out line from stringstream
		std::string info;
		lineStream >> info;
		//Reading value to the right of the  '=' which is to the right of 'chars'
		if (info == "chars")
		{
			std::string pair;
			lineStream >> pair; //line after chars = count=numberOfChars
			uint32_t spos = pair.find("="); //Find position of the "=" sign in this line
			std::string value = pair.substr(spos + 1); //Add +1 to pos in order to get position of actual value
			charCount = std::stoi(value); //Extract value from string
		}

		if (charCount > fontChars.size())
		{
			std::cout << "Fury::Render::DistanceFieldFont: ERROR! fontFile contains too many characters!" << std::endl;
			return;
		}

		if (info == "char")
		{
			//Get-Char-ID--------------------------------------
			std::string pair;
			lineStream >> pair;
			uint32_t spos = pair.find("=");
			std::string value = pair.substr(spos + 1);
			uint32_t charID = std::stoi(value);
			pair.clear();
			value.clear();
			//-------------------------------------------------

			//Get-xpos-----------------------------------------
			lineStream >> pair;
			spos = pair.find("=");
			value = pair.substr(spos + 1);
			fontChars[charID].x = std::stoi(value);
			pair.clear();
			value.clear();
			//--------------------------------------------------

			//Get-ypos-----------------------------------------
			lineStream >> pair;
			spos = pair.find("=");
			value = pair.substr(spos + 1);
			fontChars[charID].y = std::stoi(value);
			pair.clear();
			value.clear();
			//--------------------------------------------------


			//Get-width-----------------------------------------
			lineStream >> pair;
			spos = pair.find("=");
			value = pair.substr(spos + 1);
			fontChars[charID].width = std::stoi(value);
			pair.clear();
			value.clear();
			//--------------------------------------------------

			//Get-height----------------------------------------
			lineStream >> pair;
			spos = pair.find("=");
			value = pair.substr(spos + 1);
			fontChars[charID].height = std::stoi(value);
			pair.clear();
			value.clear();
			//--------------------------------------------------

			//Get-xoffset---------------------------------------
			lineStream >> pair;
			spos = pair.find("=");
			value = pair.substr(spos + 1);
			fontChars[charID].xoffset = std::stoi(value);
			pair.clear();
			value.clear();
			//--------------------------------------------------

			//Get-yoffset---------------------------------------
			lineStream >> pair;
			spos = pair.find("=");
			value = pair.substr(spos + 1);
			fontChars[charID].yoffset = std::stoi(value);
			pair.clear();
			value.clear();
			//--------------------------------------------------

			//Get-xadvance--------------------------------------
			lineStream >> pair;
			spos = pair.find("=");
			value = pair.substr(spos + 1);
			fontChars[charID].xadvance = std::stoi(value);
			pair.clear();
			value.clear();
			//--------------------------------------------------

			//Get-page------------------------------------------
			/* Not Supported at the moment */
			//--------------------------------------------------

			//Get-channel---------------------------------------
			/* Not Supported at the moment */
			//--------------------------------------------------
			/*
			std::cout << "charID: " << charID << std::endl;
			std::cout << "x: " << fontChars[charID].x << std::endl;
			std::cout << "y: " << fontChars[charID].y << std::endl;
			std::cout << "width: " << fontChars[charID].width << std::endl;
			std::cout << "height: " << fontChars[charID].height << std::endl;
			std::cout << "xoffset: " << fontChars[charID].xoffset << std::endl;
			std::cout << "yoffset: " << fontChars[charID].yoffset << std::endl;
			std::cout << "xdvance: " << fontChars[charID].xadvance << std::endl;
			*/
		}
	}
	fileBuffer.close();
}

void Demo::VimguiCore::loadImagea(const char* filepath)
{
	fontTexture = loadImage(filepath);
}

void Demo::VimguiCore::dftPrepPipelineCache()
{
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	VkResult result;
	result = vkCreatePipelineCache(logicalDevice, &pipelineCacheCreateInfo, nullptr, &dftPipelineCache);
	if (result != VK_SUCCESS)
	{
		std::cout << "VimguiCore_DistanceFieldText: Failed to create pipeline cache!" << std::endl;
	}
}

void Demo::VimguiCore::dftPrepVertexInputDescription()
{
	VkVertexInputBindingDescription vertexInputBindingDescription = {};
	vertexInputBindingDescription.binding = vertexBufferBindID;
	vertexInputBindingDescription.stride = sizeof(float) * 10;
	vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	dftBindingDescriptions.push_back(vertexInputBindingDescription);

	VkVertexInputAttributeDescription vertexInputAttributeDescription = {};
	vertexInputAttributeDescription.binding = vertexBufferBindID;
	vertexInputAttributeDescription.location = 0;
	vertexInputAttributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	vertexInputAttributeDescription.offset = 0;
	dftAttributeDescriptions.push_back(vertexInputAttributeDescription);

	vertexInputAttributeDescription.location = 1;
	vertexInputAttributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
	vertexInputAttributeDescription.offset = sizeof(float) * 4;
	dftAttributeDescriptions.push_back(vertexInputAttributeDescription);

	vertexInputAttributeDescription.location = 2;
	vertexInputAttributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	vertexInputAttributeDescription.offset = sizeof(float) * 6;
	dftAttributeDescriptions.push_back(vertexInputAttributeDescription);
}

void Demo::VimguiCore::dftPrepDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBinding = {};
	VkResult result;
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.pImmutableSamplers = nullptr;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.binding = 0;											//Descriptor Binding
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;	//Descriptor Type
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;				//Shader Stage Flag
	descriptorSetLayoutBinding.push_back(uboLayoutBinding);

	uboLayoutBinding.binding = 1;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	descriptorSetLayoutBinding.push_back(uboLayoutBinding);

	uboLayoutBinding.binding = 2;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	descriptorSetLayoutBinding.push_back(uboLayoutBinding);

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.bindingCount = descriptorSetLayoutBinding.size();
	descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBinding.data();

	result = vkCreateDescriptorSetLayout(logicalDevice, &descriptorSetLayoutCreateInfo, nullptr, &dftDescriptorSetLayout);
}

void Demo::VimguiCore::dftPrepPipelineLayout()
{
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &dftDescriptorSetLayout;
	vkCreatePipelineLayout(logicalDevice, &pipelineLayoutCreateInfo, nullptr, &dftPipelineLayout);
}

void Demo::VimguiCore::dftPrepPipeline()
{
	VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{};
	pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipelineInputAssemblyStateCreateInfo.flags = 0;
	pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo{};
	pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	pipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
	pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;
	pipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	pipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
	pipelineRasterizationStateCreateInfo.flags = 0;
	pipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	pipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;

	VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState{};
	pipelineColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	pipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	pipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	pipelineColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	pipelineColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	pipelineColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	pipelineColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
	pipelineColorBlendAttachmentState.blendEnable = VK_TRUE;

	VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
	pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	pipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
	pipelineColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
	pipelineColorBlendStateCreateInfo.blendConstants[0] = 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[1] = 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[2] = 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[3] = 0.0f;
	pipelineColorBlendStateCreateInfo.pAttachments = &pipelineColorBlendAttachmentState;
	pipelineColorBlendStateCreateInfo.attachmentCount = 1;

	VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{};
	pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	pipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
	pipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
	pipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
	pipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{};
	pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	pipelineViewportStateCreateInfo.viewportCount = 1;
	pipelineViewportStateCreateInfo.scissorCount = 1;

	VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{};
	pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	pipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	pipelineMultisampleStateCreateInfo.minSampleShading = 1.0f;
	pipelineMultisampleStateCreateInfo.pSampleMask = nullptr;
	pipelineMultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
	pipelineMultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

	std::vector<VkDynamicState> dynamicStateEnabled = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
	pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	pipelineDynamicStateCreateInfo.dynamicStateCount = dynamicStateEnabled.size();
	pipelineDynamicStateCreateInfo.pDynamicStates = dynamicStateEnabled.data();

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	VkShaderModule normalVertexShaderModule = loadShader("shaders\\2D\\sdftext\\sdf_text_normal1.vert.spv");
	VkShaderModule normalFragmentShaderModule = loadShader("shaders\\2D\\sdftext\\sdf_text_normal1.frag.spv");
	VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo{};
	pipelineShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pipelineShaderStageCreateInfo.pName = "main"; //Shader Entrypoint
	pipelineShaderStageCreateInfo.module = normalVertexShaderModule;
	pipelineShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages.push_back(pipelineShaderStageCreateInfo);

	pipelineShaderStageCreateInfo.module = normalFragmentShaderModule;
	pipelineShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages.push_back(pipelineShaderStageCreateInfo);

	VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
	pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(dftBindingDescriptions.size());
	pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(dftAttributeDescriptions.size());
	pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = dftBindingDescriptions.data();
	pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = dftAttributeDescriptions.data();

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
	graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCreateInfo.layout = dftPipelineLayout;
	graphicsPipelineCreateInfo.renderPass = renderPass;
	graphicsPipelineCreateInfo.flags = 0;
	graphicsPipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
	graphicsPipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
	graphicsPipelineCreateInfo.pViewportState = &pipelineViewportStateCreateInfo;
	graphicsPipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;
	graphicsPipelineCreateInfo.pDynamicState = &pipelineDynamicStateCreateInfo;
	graphicsPipelineCreateInfo.stageCount = shaderStages.size();
	graphicsPipelineCreateInfo.pStages = shaderStages.data();
	graphicsPipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;

	VkResult result;
	result = vkCreateGraphicsPipelines(logicalDevice, dftPipelineCache, 1, &graphicsPipelineCreateInfo, nullptr, &dftNormalPipeline);
	if (result != VK_SUCCESS)
	{
		std::cout << "VimguiCore_DistanceFieldText: Error creating normal pipeline!" << std::endl;
	}

	vkDestroyShaderModule(logicalDevice, normalVertexShaderModule, nullptr);
	vkDestroyShaderModule(logicalDevice, normalFragmentShaderModule, nullptr);
}

void Demo::VimguiCore::dftPrepDescriptorPool()
{
	std::vector<VkDescriptorPoolSize> poolSizes = {};

	VkDescriptorPoolSize descriptorPoolSize{};
	descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorPoolSize.descriptorCount = 1;
	poolSizes.push_back(descriptorPoolSize);

	descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_SAMPLER;
	descriptorPoolSize.descriptorCount = 1;
	poolSizes.push_back(descriptorPoolSize);

	descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	descriptorPoolSize.descriptorCount = 1;
	poolSizes.push_back(descriptorPoolSize);

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();
	descriptorPoolCreateInfo.poolSizeCount = poolSizes.size();
	descriptorPoolCreateInfo.maxSets = 1;

	VkResult result;
	result = vkCreateDescriptorPool(logicalDevice, &descriptorPoolCreateInfo, nullptr, &dftDescriptorPool);
	if (result != VK_SUCCESS)
	{
		std::cout << "DistanceFieldFont: Error creating descriptor pool" << std::endl;
	}
}

void Demo::VimguiCore::dftPrepDescriptorSet()
{
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = dftDescriptorPool;
	descriptorSetAllocateInfo.pSetLayouts = &dftDescriptorSetLayout;
	descriptorSetAllocateInfo.descriptorSetCount = 1;

	VkResult result;
	result = vkAllocateDescriptorSets(logicalDevice, &descriptorSetAllocateInfo, &dftDescriptor);

	VkDescriptorImageInfo descriptorImageInfo{};
	descriptorImageInfo.sampler = fontTexture->textureDescriptor.sampler;
	descriptorImageInfo.imageView = fontTexture->textureDescriptor.imageView;
	descriptorImageInfo.imageLayout = fontTexture->textureDescriptor.imageLayout;

	std::vector<VkWriteDescriptorSet> writeDescriptorSets = {};

	VkWriteDescriptorSet writeDescriptorSet{};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.dstSet = dftDescriptor;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSet.dstBinding = 0;
	writeDescriptorSet.pBufferInfo = &uboBuffer->descriptor;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSets.push_back(writeDescriptorSet);

	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	writeDescriptorSet.dstBinding = 1;
	writeDescriptorSet.pImageInfo = &fontTexture->textureDescriptor; //NOTE pIMAGEINFO
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSets.push_back(writeDescriptorSet);

	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	writeDescriptorSet.dstBinding = 2;
	writeDescriptorSet.pImageInfo = &fontTexture->textureDescriptor;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSets.push_back(writeDescriptorSet);

	vkUpdateDescriptorSets(logicalDevice, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);
}

void Demo::VimguiCore::dftPrepDistanceFieldTextBuffer()
{
	//Create a size vector that represents our buffers maximum size
	std::vector<float> sizeVector;
	//Resize vector so buffer can hold "maxElements"
	sizeVector.resize(dftMaxElements); //Each vertex is represented by 7 floats, each element is represented by 6 vertices.
	//Calculate buffer size
	size_t vertexBufferSize = sizeVector.size();
	//Generate buffer
	dftBuffer = new UIBuffer(logicalDevice, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBufferSize);
	createBuffer(dftBuffer);
	//Map buffer memory, storing pointer to mapped memory in buffer container.
	dftBuffer->map();
	//Cleaning up size vector
	sizeVector.resize(0);
	sizeVector.clear();
	sizeVector.shrink_to_fit();
}
//------------------------------------------------------------------------------------------------


//Color-Line-Private-Funcs------------------------------------------------------------------------
/* Note: */
void Demo::VimguiCore::clPrepPipelineCache()
{
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	VkResult result;
	result = vkCreatePipelineCache(logicalDevice, &pipelineCacheCreateInfo, nullptr, &clPipelineCache);
	if (result != VK_SUCCESS)
	{
		std::cout << "VimguiCore_ColorLine: Failed to create pipeline cache!" << std::endl;
	}
}

void Demo::VimguiCore::clPrepVertexInputDescription()
{
	VkVertexInputBindingDescription vertexInputBindingDescription = {};
	vertexInputBindingDescription.binding = vertexBufferBindID;
	vertexInputBindingDescription.stride = sizeof(float) * 8;
	vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	clBindingDescriptions.push_back(vertexInputBindingDescription);

	VkVertexInputAttributeDescription vertexInputAttributeDescription = {};
	vertexInputAttributeDescription.binding = vertexBufferBindID;
	vertexInputAttributeDescription.location = 0;
	vertexInputAttributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	vertexInputAttributeDescription.offset = 0;
	clAttributeDescriptions.push_back(vertexInputAttributeDescription);

	vertexInputAttributeDescription.location = 1;
	vertexInputAttributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	vertexInputAttributeDescription.offset = sizeof(float) * 4;
	clAttributeDescriptions.push_back(vertexInputAttributeDescription);

}

void Demo::VimguiCore::clPrepDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBinding = {};
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.pImmutableSamplers = nullptr;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.binding = 0;											//Descriptor Binding
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;	//Descriptor Type
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;				//Shader Stage Flag
	descriptorSetLayoutBinding.push_back(uboLayoutBinding);
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.bindingCount = descriptorSetLayoutBinding.size();
	descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBinding.data();
	VkResult result;
	result = vkCreateDescriptorSetLayout(logicalDevice, &descriptorSetLayoutCreateInfo, nullptr, &clDescriptorSetLayout);
}

void Demo::VimguiCore::clPrepPipelineLayout()
{
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &clDescriptorSetLayout;
	vkCreatePipelineLayout(logicalDevice, &pipelineLayoutCreateInfo, nullptr, &clPipelineLayout);
}

void Demo::VimguiCore::clPrepPipeline()
{
	VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{};
	pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	pipelineInputAssemblyStateCreateInfo.flags = 0;
	pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo{};
	pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	pipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
	pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;
	pipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	pipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
	pipelineRasterizationStateCreateInfo.flags = 0;
	pipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
	pipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;

	VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState{};
	pipelineColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	pipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	pipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	pipelineColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	pipelineColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	pipelineColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	pipelineColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
	pipelineColorBlendAttachmentState.blendEnable = VK_TRUE;

	VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
	pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	pipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
	pipelineColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
	pipelineColorBlendStateCreateInfo.blendConstants[0] = 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[1] = 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[2] = 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[3] = 0.0f;
	pipelineColorBlendStateCreateInfo.pAttachments = &pipelineColorBlendAttachmentState;
	pipelineColorBlendStateCreateInfo.attachmentCount = 1;

	VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{};
	pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	pipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
	pipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
	pipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
	pipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{};
	pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	pipelineViewportStateCreateInfo.viewportCount = 1;
	pipelineViewportStateCreateInfo.scissorCount = 1;

	VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{};
	pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	pipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	pipelineMultisampleStateCreateInfo.minSampleShading = 1.0f;
	pipelineMultisampleStateCreateInfo.pSampleMask = nullptr;
	pipelineMultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
	pipelineMultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

	std::vector<VkDynamicState> dynamicStateEnabled = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
	pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	pipelineDynamicStateCreateInfo.dynamicStateCount = dynamicStateEnabled.size();
	pipelineDynamicStateCreateInfo.pDynamicStates = dynamicStateEnabled.data();

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	VkShaderModule vertexShaderModule = loadShader("shaders\\2D\\line\\line.vert.spv");
	VkShaderModule fragmentShaderModule = loadShader("shaders\\2D\\line\\line.frag.spv");
	VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo{};
	pipelineShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pipelineShaderStageCreateInfo.pName = "main"; //Shader Entrypoint
	pipelineShaderStageCreateInfo.module = vertexShaderModule;
	pipelineShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages.push_back(pipelineShaderStageCreateInfo);

	pipelineShaderStageCreateInfo.module = fragmentShaderModule;
	pipelineShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages.push_back(pipelineShaderStageCreateInfo);

	VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
	pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(clBindingDescriptions.size());
	pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(clAttributeDescriptions.size());
	pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = clBindingDescriptions.data();
	pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = clAttributeDescriptions.data();

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
	graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCreateInfo.layout = clPipelineLayout;
	graphicsPipelineCreateInfo.renderPass = renderPass;
	graphicsPipelineCreateInfo.flags = 0;
	graphicsPipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
	graphicsPipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
	graphicsPipelineCreateInfo.pViewportState = &pipelineViewportStateCreateInfo;
	graphicsPipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;
	graphicsPipelineCreateInfo.pDynamicState = &pipelineDynamicStateCreateInfo;
	graphicsPipelineCreateInfo.stageCount = shaderStages.size();
	graphicsPipelineCreateInfo.pStages = shaderStages.data();
	graphicsPipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;

	VkResult result;
	result = vkCreateGraphicsPipelines(logicalDevice, clPipelineCache, 1, &graphicsPipelineCreateInfo, nullptr, &clPipeline);
	if (result != VK_SUCCESS)
	{
		std::cout << "DistanceFieldFont: Error creating normal pipeline!" << std::endl;
	}

	vkDestroyShaderModule(logicalDevice, vertexShaderModule, nullptr);
	vkDestroyShaderModule(logicalDevice, fragmentShaderModule, nullptr);
}

void Demo::VimguiCore::clPrepDescriptorPool()
{
	std::vector<VkDescriptorPoolSize> poolSizes = {};

	VkDescriptorPoolSize descriptorPoolSize{};
	descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorPoolSize.descriptorCount = 1;
	poolSizes.push_back(descriptorPoolSize);

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();
	descriptorPoolCreateInfo.poolSizeCount = poolSizes.size();
	descriptorPoolCreateInfo.maxSets = 1;

	VkResult result;
	result = vkCreateDescriptorPool(logicalDevice, &descriptorPoolCreateInfo, nullptr, &clDescriptorPool);
	if (result != VK_SUCCESS)
	{
		std::cout << "VimguiCore_ColorLine: Error creating descriptor pool" << std::endl;
	}
}

void Demo::VimguiCore::clPrepDescriptorSet()
{
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = clDescriptorPool;
	descriptorSetAllocateInfo.pSetLayouts = &clDescriptorSetLayout;
	descriptorSetAllocateInfo.descriptorSetCount = 1;

	VkResult result;
	result = vkAllocateDescriptorSets(logicalDevice, &descriptorSetAllocateInfo, &clDescriptor);

	std::vector<VkWriteDescriptorSet> writeDescriptorSets = {};
	VkWriteDescriptorSet writeDescriptorSet{};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.dstSet = clDescriptor;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSet.dstBinding = 0;
	writeDescriptorSet.pBufferInfo = &uboBuffer->descriptor;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSets.push_back(writeDescriptorSet);

	vkUpdateDescriptorSets(logicalDevice, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);
}

void Demo::VimguiCore::clPrepColorLineBuffer()
{
	//Create a size vector that represents our buffers maximum size
	std::vector<float> sizeVector;
	//Resize vector so buffer can hold "maxElements"
	sizeVector.resize(clMaxElements); //Each vertex is represented by 7 floats, each element is represented by 6 vertices.
	//Calculate buffer size
	size_t vertexBufferSize = sizeVector.size();
	//Generate buffer
	clBuffer = new UIBuffer(logicalDevice, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBufferSize);
	createBuffer(clBuffer);
	//Map buffer memory, storing pointer to mapped memory in buffer container.
	clBuffer->map();
	//Cleaning up size vector
	sizeVector.resize(0);
	sizeVector.clear();
	sizeVector.shrink_to_fit();
	/*
	*	Buffer is now prepared, and can be written to as easy as;
	*	size_t size = colorQuadVertices.size() * sizeof(float);
	*	memcpy(colorQuadBuffer->mapped, colorQuadVertices.data(), size);
	*/
}
//------------------------------------------------------------------------------------------------

Demo::Vimgui::Vimgui(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, uint32_t swapWidth, uint32_t swapHeight, VkRenderPass renderPass, uint32_t graphicsQueueIndex, VkQueue graphicsQueue)
{
	this->vimguiCore = new VimguiCore(physicalDevice, logicalDevice, swapWidth, swapHeight, renderPass, graphicsQueueIndex, graphicsQueue);
}

Demo::Vimgui::~Vimgui()
{
	delete vimguiCore;
}

void Demo::Vimgui::kbInput(uint32_t key, uint32_t additional)
{
}

void Demo::Vimgui::mouseInput(int32_t posx, int32_t posy)
{
}

void Demo::Vimgui::mouseKeyInput(uint32_t key, uint32_t additional)
{
}

uint32_t Demo::Vimgui::genPanel(Panel panel)
{
	if (panelID == 0)
	{
		panelID++;
	}
	panel.panelID = panelID;

	if (panel.panelBody)
	{
		ColorQuad panelbdy;
		panelbdy.color = panel.bodyColor;
		panelbdy.position.x = panel.posx;
		panelbdy.position.z = panel.defaultLayer;
		panel.panelbodyPos.x = panel.posx;
		panel.panelbodyPos.z = panel.defaultLayer;
		if (panel.panelHeader)
		{
			panelbdy.position.y = panel.posy + panel.headerHeight;
			panelbdy.size.y = panel.height - panel.headerHeight;
			panel.panelbodyPos.y = panel.posy + panel.headerHeight;
		}
		else
		{
			panelbdy.position.y = panel.posy;
			panelbdy.size.y = panel.height;
			panel.panelbodyPos.y = panel.posy;
		}
		panelbdy.size.x = panel.width;
		panel.panelbodyid = vimguiCore->addColorQuad(panelbdy);
		panel.colorQuadIDs.push_back(panel.panelbodyid);
	}

	if (panel.panelHeader)
	{
		ColorQuad panelheader;
		panelheader.color = panel.headerColor;
		panelheader.position.x = panel.posx;
		panelheader.position.y = panel.posy;
		panelheader.position.z = panel.defaultLayer;
		panelheader.size.x = panel.width;
		panelheader.size.y = panel.headerHeight;
		panel.panelheaderPos.x = panel.posx;
		panel.panelheaderPos.y = panel.posy;
		panel.panelheaderPos.z = panel.defaultLayer;

		panel.panelheaderid = vimguiCore->addColorQuad(panelheader);
		panel.colorQuadIDs.push_back(panel.panelheaderid);
	}

	if (panel.panelTitle)
	{
		DistanceFieldText paneltitle;
		paneltitle.color = panel.titleColor;
		paneltitle.position.x = panel.posx + 10.0f;
		paneltitle.position.y = panel.posy + 5.0f;
		paneltitle.position.z = panel.defaultLayer - 0.001f;
		paneltitle.text = panel.panelTitleString;
		paneltitle.scale = 0.8f;
		panel.panelTitlePos.x = panel.posx + 10.0f;
		panel.panelTitlePos.y = panel.posy + 5.0f;
		panel.panelTitlePos.z = panel.defaultLayer - 0.001f;

		panel.paneltitleid = vimguiCore->addDistanceFieldText(paneltitle);
		panel.distanceFieldTextIDs.push_back(panel.paneltitleid);

	}

	if (panel.panelOutline)
	{
		ColorLine colorLine;
		colorLine.p1Pos.x = panel.posx;
		colorLine.p1Pos.y = panel.posy;
		colorLine.p1Pos.z = panel.defaultLayer - 0.002f;
		colorLine.p1Color = glm::vec4(255, 255, 255, 255);
		colorLine.p2Pos.x = panel.posx;
		colorLine.p2Pos.y = panel.posy + panel.width;
		colorLine.p2Pos.z = panel.defaultLayer - 0.002f;
		colorLine.p2Color = glm::vec4(255, 255, 255, 255);
		
		panel.outlinetopid = vimguiCore->addColorLine(colorLine);
		panel.colorLineIDs.push_back(panel.outlinetopid);
	}

	if (panel.panelControls)
	{
		//TODO; Add controls
		//exit minimize etc.
	}


	panels.push_back(panel);
	panelID++;
	return panel.panelID;
}

void Demo::Vimgui::destroyPanel(uint32_t panelID)
{
	for (uint32_t i = 0; i < panels.size(); i++)
	{
		if (panels[i].panelID == panelID)
		{
			for (uint32_t j = 0; j < panels[i].colorLineIDs.size(); j++)
			{
				vimguiCore->removeColorLine(panels[i].colorLineIDs[j]);
				vimguiCore->removeColorLineElement(panels[i].colorLineIDs[j]);
			}

			for (uint32_t k = 0; k < panels[i].colorQuadIDs.size(); k++)
			{
				vimguiCore->removeColorQuad(panels[i].colorQuadIDs[k]);
				vimguiCore->removeColorQuadElement(panels[i].colorQuadIDs[k]);
			}

			for (uint32_t l = 0; l < panels[i].distanceFieldTextIDs.size(); l++)
			{
				vimguiCore->removeDistanceFieldText(panels[i].distanceFieldTextIDs[l]);
				vimguiCore->removeDistanceFieldTextElement(panels[i].distanceFieldTextIDs[l]);
			}

			//TODO, ERASE VECTOR ELEMENTS
		}
	}


}

void Demo::Vimgui::destroyAllPanels()
{
	for (uint32_t i = 0; i < panels.size(); i++)
	{
		for (uint32_t j = 0; j < panels[i].colorLineIDs.size(); j++)
		{
			vimguiCore->removeColorLine(panels[i].colorLineIDs[j]);
			vimguiCore->removeColorLineElement(panels[i].colorLineIDs[j]);
		}

		for (uint32_t k = 0; k < panels[i].colorQuadIDs.size(); k++)
		{
			vimguiCore->removeColorQuad(panels[i].colorQuadIDs[k]);
			vimguiCore->removeColorQuadElement(panels[i].colorQuadIDs[k]);
		}

		for (uint32_t l = 0; l < panels[i].distanceFieldTextIDs.size(); l++)
		{
			vimguiCore->removeDistanceFieldText(panels[i].distanceFieldTextIDs[l]);
			vimguiCore->removeDistanceFieldTextElement(panels[i].distanceFieldTextIDs[l]);
		}
	}
}

uint32_t Demo::Vimgui::findPanelbody(uint32_t posx, uint32_t posy)
{
	for (uint32_t i = 0; i < panels.size(); i++)
	{
		if (posx > panels[i].panelbodyPos.x && posx < panels[i].panelbodyPos.x + panels[i].width)
		{
			if (posy > panels[i].panelbodyPos.y&& posy < panels[i].panelbodyPos.y + panels[i].height)
			{
				return panels[i].panelID;
			}
		}
	}
	return 0;
}

uint32_t Demo::Vimgui::findPanelheader(uint32_t posx, uint32_t posy)
{
	for (uint32_t i = 0; i < panels.size(); i++)
	{
		if (posx > panels[i].panelheaderPos.x && posx < panels[i].panelheaderPos.x + panels[i].width)
		{
			if (posy > panels[i].panelheaderPos.y && posy < panels[i].panelheaderPos.y + panels[i].headerHeight)
			{
				return panels[i].panelID;
			}
		}
	}
	return 0;
}

void Demo::Vimgui::resizePanel(uint32_t panelID, uint32_t width, uint32_t height)
{
	//Find old panel>create Panel panel copy>remove old, add new.
}

void Demo::Vimgui::movePanel(uint32_t panelID, int32_t x, int32_t y)
{
	//Go through all elements bound to panel, and move them by a value
	for (uint32_t i = 0; i < panels.size(); i++)
	{
		if (panels[i].panelID == panelID)
		{
			//Panel with target ID found
			//Reposition header, title, body, controls
			panels[i].panelbodyPos.x = panels[i].panelbodyPos.x + x;
			panels[i].panelbodyPos.y = panels[i].panelbodyPos.y + y;

			panels[i].panelheaderPos.x = panels[i].panelheaderPos.x + x;
			panels[i].panelheaderPos.y = panels[i].panelheaderPos.y + y;

			panels[i].panelTitlePos.x = panels[i].panelTitlePos.x + x;
			panels[i].panelTitlePos.y = panels[i].panelTitlePos.y + y;

			vimguiCore->positionColorQuad(panels[i].panelbodyid, panels[i].panelbodyPos.x, panels[i].panelbodyPos.y);
			vimguiCore->positionColorQuad(panels[i].panelheaderid, panels[i].panelheaderPos.x, panels[i].panelheaderPos.y);
			vimguiCore->positionDistanceFieldText(panels[i].paneltitleid, panels[i].panelTitlePos.x, panels[i].panelTitlePos.y);
		}
	}
}

void Demo::Vimgui::dragPanel(int32_t x, int32_t y)
{
	//find & move in one func
}

void Demo::Vimgui::addComponent(uint32_t COMPONENT, uint32_t panelID)
{
}

void Demo::Vimgui::addComponent(DragFloat3 df3Comp, uint32_t panelID)
{

}

void Demo::Vimgui::updatePanel(uint32_t panelID)
{
}

void Demo::Vimgui::hightLightPanel(uint32_t panelID)
{
}

void Demo::Vimgui::focusPanel(uint32_t panelID)
{
}

void Demo::Vimgui::hidePanel(uint32_t panelID)
{
}

void Demo::Vimgui::setPanelOpacity(uint32_t panelID)
{
}

void Demo::Vimgui::pushForwards(uint32_t panelID)
{
}

void Demo::Vimgui::pushBackwards(uint32_t panelID)
{
}

void Demo::Vimgui::regenerateUserInterface()
{
	vimguiCore->rebuildColorLineBuffer();
	vimguiCore->rebuildColorQuadBuffer();
	vimguiCore->rebuildDistanceFieldTextBuffer();
}

void Demo::Vimgui::itsShowtime(float dt)
{

}
