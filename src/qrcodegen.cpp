/* 
 * QR Code generator library (C++)
 * 
 * Copyright (c) Project Nayuki. (MIT License)
 * https://www.nayuki.io/page/qr-code-generator-library
 */

#include "grm/qrcodegen.hpp"
#include <algorithm>
#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <utility>

using std::int8_t;
using std::uint8_t;
using std::size_t;
using std::vector;

namespace qrcodegen {

/*---- Class QrSegment ----*/

QrSegment::Mode::Mode(int mode, int cc0, int cc1, int cc2) :
		modeBits(mode) {
	numBitsCharCount[0] = cc0;
	numBitsCharCount[1] = cc1;
	numBitsCharCount[2] = cc2;
}

int QrSegment::Mode::getModeBits() const {
	return modeBits;
}

int QrSegment::Mode::numCharCountBits(int ver) const {
	return numBitsCharCount[(ver + 7) / 17];
}

const QrSegment::Mode QrSegment::Mode::NUMERIC     (0x1, 10, 12, 14);
const QrSegment::Mode QrSegment::Mode::ALPHANUMERIC(0x2,  9, 11, 13);
const QrSegment::Mode QrSegment::Mode::BYTE        (0x4,  8, 16, 16);
const QrSegment::Mode QrSegment::Mode::KANJI       (0x8,  8, 10, 12);
const QrSegment::Mode QrSegment::Mode::ECI         (0x7,  0,  0,  0);

QrSegment QrSegment::makeBytes(const vector<uint8_t> &data) {
	if (data.size() > static_cast<unsigned int>(INT_MAX))
		throw std::length_error("Data too long");
	BitBuffer bb;
	for (uint8_t b : data)
		bb.appendBits(b, 8);
	return QrSegment(Mode::BYTE, static_cast<int>(data.size()), std::move(bb));
}

static const char *ALPHANUMERIC_CHARSET = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:";

QrSegment QrSegment::makeNumeric(const char *digits) {
	BitBuffer bb;
	int accumData = 0;
	int accumCount = 0;
	int charCount = 0;
	for (; *digits != '\0'; digits++, charCount++) {
		char c = *digits;
		if (c < '0' || c > '9')
			throw std::domain_error("String contains non-numeric characters");
		accumData = accumData * 10 + (c - '0');
		accumCount++;
		if (accumCount == 3) {
			bb.appendBits(static_cast<uint32_t>(accumData), 10);
			accumData = 0;
			accumCount = 0;
		}
	}
	if (accumCount > 0)
		bb.appendBits(static_cast<uint32_t>(accumData), accumCount * 3 + 1);
	return QrSegment(Mode::NUMERIC, charCount, std::move(bb));
}

QrSegment QrSegment::makeAlphanumeric(const char *text) {
	BitBuffer bb;
	int accumData = 0;
	int accumCount = 0;
	int charCount = 0;
	for (; *text != '\0'; text++, charCount++) {
		const char *temp = std::strchr(ALPHANUMERIC_CHARSET, *text);
		if (temp == nullptr)
			throw std::domain_error("String contains unencodable characters in alphanumeric mode");
		accumData = accumData * 45 + static_cast<int>(temp - ALPHANUMERIC_CHARSET);
		accumCount++;
		if (accumCount == 2) {
			bb.appendBits(static_cast<uint32_t>(accumData), 11);
			accumData = 0;
			accumCount = 0;
		}
	}
	if (accumCount > 0)
		bb.appendBits(static_cast<uint32_t>(accumData), 6);
	return QrSegment(Mode::ALPHANUMERIC, charCount, std::move(bb));
}

vector<QrSegment> QrSegment::makeSegments(const char *text) {
	vector<QrSegment> result;
	if (*text == '\0') {
		// Empty
	} else if (isNumeric(text)) {
		result.push_back(makeNumeric(text));
	} else if (isAlphanumeric(text)) {
		result.push_back(makeAlphanumeric(text));
	} else {
		vector<uint8_t> bytes;
		for (; *text != '\0'; text++)
			bytes.push_back(static_cast<uint8_t>(*text));
		result.push_back(makeBytes(bytes));
	}
	return result;
}

QrSegment QrSegment::makeEci(long assignVal) {
	BitBuffer bb;
	if (assignVal < 0)
		throw std::domain_error("ECI assignment value out of range");
	else if (assignVal < (1 << 7))
		bb.appendBits(static_cast<uint32_t>(assignVal), 8);
	else if (assignVal < (1 << 14)) {
		bb.appendBits(2, 2);
		bb.appendBits(static_cast<uint32_t>(assignVal), 14);
	} else if (assignVal < 1000000L) {
		bb.appendBits(6, 3);
		bb.appendBits(static_cast<uint32_t>(assignVal), 21);
	} else
		throw std::domain_error("ECI assignment value out of range");
	return QrSegment(Mode::ECI, 0, std::move(bb));
}

QrSegment::QrSegment(const Mode &md, int numCh, const std::vector<bool> &dt) :
		mode(&md),
		numChars(numCh),
		data(dt) {
	if (numCh < 0)
		throw std::domain_error("Invalid value");
}

QrSegment::QrSegment(const Mode &md, int numCh, std::vector<bool> &&dt) :
		mode(&md),
		numChars(numCh),
		data(std::move(dt)) {
	if (numCh < 0)
		throw std::domain_error("Invalid value");
}

int QrSegment::getTotalBits(const vector<QrSegment> &segs, int version) {
	int result = 0;
	for (const QrSegment &seg : segs) {
		int ccbits = seg.mode->numCharCountBits(version);
		if (seg.numChars >= (1L << ccbits))
			return -1;
		if (4 + ccbits > INT_MAX - result)
			return -1;
		result += 4 + ccbits;
		if (seg.data.size() > static_cast<unsigned int>(INT_MAX - result))
			return -1;
		result += static_cast<int>(seg.data.size());
	}
	return result;
}

bool QrSegment::isNumeric(const char *text) {
	for (; *text != '\0'; text++) {
		char c = *text;
		if (c < '0' || c > '9')
			return false;
	}
	return true;
}

bool QrSegment::isAlphanumeric(const char *text) {
	for (; *text != '\0'; text++) {
		if (std::strchr(ALPHANUMERIC_CHARSET, *text) == nullptr)
			return false;
	}
	return true;
}

const QrSegment::Mode &QrSegment::getMode() const {
	return *mode;
}

int QrSegment::getNumChars() const {
	return numChars;
}

const std::vector<bool> &QrSegment::getData() const {
	return data;
}

/*---- Class QrCode ----*/

QrCode QrCode::encodeText(const char *text, Ecc ecl) {
	vector<QrSegment> segs = QrSegment::makeSegments(text);
	return encodeSegments(segs, ecl);
}

QrCode QrCode::encodeBinary(const vector<uint8_t> &data, Ecc ecl) {
	vector<QrSegment> segs{QrSegment::makeBytes(data)};
	return encodeSegments(segs, ecl);
}

QrCode QrCode::encodeSegments(const vector<QrSegment> &segs, Ecc ecl,
		int minVersion, int maxVersion, int mask, bool boostEcl) {
	if (!(MIN_VERSION <= minVersion && minVersion <= maxVersion && maxVersion <= MAX_VERSION) || mask < -1 || mask > 7)
		throw std::invalid_argument("Invalid value");
	
	int version, dataUsedBits;
	for (version = minVersion; ; version++) {
		int dataCapacityBits = getNumDataCodewords(version, ecl) * 8;
		dataUsedBits = QrSegment::getTotalBits(segs, version);
		if (dataUsedBits != -1 && dataUsedBits <= dataCapacityBits)
			break;
		if (version >= maxVersion) {
			std::ostringstream sb;
			if (dataUsedBits == -1)
				sb << "Segment too long";
			else {
				sb << "Data length = " << dataUsedBits << " bits, ";
				sb << "Max capacity = " << dataCapacityBits << " bits";
			}
			throw data_too_long(sb.str());
		}
	}
	
	if (boostEcl) {
		for (Ecc newEcl : {Ecc::MEDIUM, Ecc::QUARTILE, Ecc::HIGH}) {
			if (dataUsedBits <= getNumDataCodewords(version, newEcl) * 8)
				ecl = newEcl;
		}
	}
	
	BitBuffer bb;
	for (const QrSegment &seg : segs) {
		bb.appendBits(static_cast<uint32_t>(seg.getMode().getModeBits()), 4);
		bb.appendBits(static_cast<uint32_t>(seg.getNumChars()), seg.getMode().numCharCountBits(version));
		for (bool b : seg.getData())
			bb.push_back(b);
	}
	assert(bb.size() == static_cast<unsigned int>(dataUsedBits));
	
	int dataCapacityBits = getNumDataCodewords(version, ecl) * 8;
	assert(bb.size() <= static_cast<unsigned int>(dataCapacityBits));
	bb.appendBits(0, std::min(4, dataCapacityBits - static_cast<int>(bb.size())));
	bb.appendBits(0, (8 - static_cast<int>(bb.size()) % 8) % 8);
	assert(bb.size() % 8 == 0);
	
	for (uint8_t padByte = 0xEC; bb.size() < static_cast<unsigned int>(dataCapacityBits); padByte ^= 0xEC ^ 0x11)
		bb.appendBits(padByte, 8);
	
	vector<uint8_t> dataCodewords(bb.size() / 8);
	for (size_t i = 0; i < bb.size(); i++)
		dataCodewords[i >> 3] |= (bb[i] ? 1 : 0) << (7 - (i & 7));
	
	return QrCode(version, ecl, dataCodewords, mask);
}

QrCode::QrCode(int ver, Ecc ecl, const vector<uint8_t> &dataCodewords, int msk) :
		version(ver),
		size(ver * 4 + 17),
		errorCorrectionLevel(ecl),
		modules(size, vector<bool>(size)),
		isFunction(size, vector<bool>(size)) {
	if (ver < MIN_VERSION || ver > MAX_VERSION)
		throw std::domain_error("Version value out of range");
	if (msk < -1 || msk > 7)
		throw std::domain_error("Mask value out of range");
	
	drawFunctionPatterns();
	const vector<uint8_t> allCodewords = addEccAndInterleave(dataCodewords);
	drawCodewords(allCodewords);
	
	if (msk == -1) {
		long minPenalty = LONG_MAX;
		for (int i = 0; i < 8; i++) {
			applyMask(i);
			drawFormatBits(i);
			long penalty = getPenaltyScore();
			if (penalty < minPenalty) {
				msk = i;
				minPenalty = penalty;
			}
			applyMask(i);
		}
	}
	assert(0 <= msk && msk <= 7);
	mask = msk;
	applyMask(msk);
	drawFormatBits(msk);
	isFunction.clear();
	isFunction.shrink_to_fit();
}

int QrCode::getVersion() const {
	return version;
}

int QrCode::getSize() const {
	return size;
}

QrCode::Ecc QrCode::getErrorCorrectionLevel() const {
	return errorCorrectionLevel;
}

int QrCode::getMask() const {
	return mask;
}

bool QrCode::getModule(int x, int y) const {
	return 0 <= x && x < size && 0 <= y && y < size && module(x, y);
}

std::string QrCode::toSvgString(int border) const {
	if (border < 0)
		throw std::domain_error("Border must be non-negative");
	if (border > INT_MAX / 2 || border * 2 > INT_MAX - size)
		throw std::length_error("Border too large");
	
	const int fullSize = size + border * 2;
	std::ostringstream sb;
	sb << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	sb << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"512\" height=\"512\" viewBox=\"0 0 ";
	sb << fullSize << " " << fullSize << "\" stroke=\"none\" shape-rendering=\"crispEdges\">\n";
	sb << "\t<rect width=\"100%\" height=\"100%\" fill=\"#FFFFFF\"/>\n";
	sb << "\t<path d=\"";
	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			if (getModule(x, y)) {
				if (x != 0 || y != 0)
					sb << " ";
				sb << "M" << (x + border) << "," << (y + border) << "h1v1h-1z";
			}
		}
	}
	sb << "\" fill=\"#000000\"/>\n";
	sb << "</svg>\n";
	return sb.str();
}

void QrCode::drawFunctionPatterns() {
	for (int i = 0; i < size; i++) {
		setFunctionModule(6, i, i % 2 == 0);
		setFunctionModule(i, 6, i % 2 == 0);
	}
	
	drawFinderPattern(3, 3);
	drawFinderPattern(size - 4, 3);
	drawFinderPattern(3, size - 4);
	
	const int numAlign = getAlignmentPatternPositions();
	for (int i = 0; i < numAlign; i++) {
		for (int j = 0; j < numAlign; j++) {
			if (!((i == 0 && j == 0) || (i == 0 && j == numAlign - 1) || (i == numAlign - 1 && j == 0))) {
				int posI = (i == 0 ? 6 : size - 7 - (numAlign - 1 - i) * (size >= 116 ? 28 : (size >= 84 ? 26 : 24)));
				int posJ = (j == 0 ? 6 : size - 7 - (numAlign - 1 - j) * (size >= 116 ? 28 : (size >= 84 ? 26 : 24)));
				drawAlignmentPattern(posI, posJ);
			}
		}
	}
	
	drawFormatBits(0);
	drawVersion();
}

void QrCode::drawFormatBits(int msk) {
	static const int table[] = {1, 0, 3, 2};
	int data = table[static_cast<int>(errorCorrectionLevel)] << 3 | msk;
	int rem = data;
	for (int i = 0; i < 10; i++)
		rem = (rem << 1) ^ ((rem >> 9) * 0x537);
	int bits = (data << 10 | rem) ^ 0x5412;
	assert(bits >> 15 == 0);
	
	for (int i = 0; i <= 5; i++)
		setFunctionModule(8, i, ((bits >> i) & 1) != 0);
	setFunctionModule(8, 7, ((bits >> 6) & 1) != 0);
	setFunctionModule(8, 8, ((bits >> 7) & 1) != 0);
	setFunctionModule(7, 8, ((bits >> 8) & 1) != 0);
	for (int i = 9; i < 15; i++)
		setFunctionModule(14 - i, 8, ((bits >> i) & 1) != 0);
	
	for (int i = 0; i < 8; i++)
		setFunctionModule(size - 1 - i, 8, ((bits >> i) & 1) != 0);
	for (int i = 8; i < 15; i++)
		setFunctionModule(8, size - 15 + i, ((bits >> i) & 1) != 0);
	setFunctionModule(8, size - 8, true);
}

void QrCode::drawVersion() {
	if (version < 7)
		return;
	
	int rem = version;
	for (int i = 0; i < 12; i++)
		rem = (rem << 1) ^ ((rem >> 11) * 0x1F25);
	long bits = static_cast<long>(version) << 12 | rem;
	assert(bits >> 18 == 0);
	
	for (int i = 0; i < 18; i++) {
		bool bit = ((bits >> i) & 1) != 0;
		int a = size - 11 + i % 3;
		int b = i / 3;
		setFunctionModule(a, b, bit);
		setFunctionModule(b, a, bit);
	}
}

void QrCode::drawFinderPattern(int x, int y) {
	for (int dy = -4; dy <= 4; dy++) {
		for (int dx = -4; dx <= 4; dx++) {
			int dist = std::max(std::abs(dx), std::abs(dy));
			int xx = x + dx, yy = y + dy;
			if (0 <= xx && xx < size && 0 <= yy && yy < size)
				setFunctionModule(xx, yy, dist != 2 && dist != 4);
		}
	}
}

void QrCode::drawAlignmentPattern(int x, int y) {
	for (int dy = -2; dy <= 2; dy++) {
		for (int dx = -2; dx <= 2; dx++)
			setFunctionModule(x + dx, y + dy, std::max(std::abs(dx), std::abs(dy)) != 1);
	}
}

void QrCode::setFunctionModule(int x, int y, bool isBlack) {
	modules[y][x] = isBlack;
	isFunction[y][x] = true;
}

bool QrCode::module(int x, int y, bool defaultVal) const {
	return (0 <= x && x < size && 0 <= y && y < size) ? modules[y][x] : defaultVal;
}

vector<uint8_t> QrCode::addEccAndInterleave(const vector<uint8_t> &data) const {
	size_t numBlocks = NUM_ERROR_CORRECTION_BLOCKS[static_cast<int>(errorCorrectionLevel)][version];
	size_t blockEccLen = ECC_CODEWORDS_PER_BLOCK[static_cast<int>(errorCorrectionLevel)][version];
	size_t rawCodewords = getNumRawDataModules(version) / 8;
	size_t numShortBlocks = numBlocks - rawCodewords % numBlocks;
	size_t shortBlockLen = rawCodewords / numBlocks;
	
	vector<vector<uint8_t>> blocks;
	const vector<uint8_t> rsDiv = reedSolomonComputeDivisor(static_cast<int>(blockEccLen));
	for (size_t i = 0, k = 0; i < numBlocks; i++) {
		vector<uint8_t> dat(data.cbegin() + k, data.cbegin() + (k + shortBlockLen - blockEccLen + (i >= numShortBlocks ? 1 : 0)));
		k += dat.size();
		const vector<uint8_t> ecc = reedSolomonComputeRemainder(dat, rsDiv);
		dat.insert(dat.end(), ecc.cbegin(), ecc.cend());
		blocks.push_back(std::move(dat));
	}
	
	vector<uint8_t> result;
	for (size_t i = 0; i < blocks[0].size(); i++) {
		for (size_t j = 0; j < blocks.size(); j++) {
			if (i != shortBlockLen - blockEccLen || j >= numShortBlocks)
				result.push_back(blocks[j][i]);
		}
	}
	return result;
}

void QrCode::drawCodewords(const vector<uint8_t> &data) {
	size_t i = 0;
	for (int right = size - 1; right >= 1; right -= 2) {
		if (right == 6)
			right = 5;
		for (int vert = 0; vert < size; vert++) {
			for (int j = 0; j < 2; j++) {
				int x = right - j;
				bool upward = ((right + 1) & 2) == 0;
				int y = upward ? size - 1 - vert : vert;
				if (!isFunction[y][x] && i < data.size() * 8) {
					modules[y][x] = ((data[i >> 3] >> (7 - (i & 7))) & 1) != 0;
					i++;
				}
			}
		}
	}
	assert(i == data.size() * 8);
}

void QrCode::applyMask(int msk) {
	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			bool invert;
			switch (msk) {
				case 0:  invert = (x + y) % 2 == 0;                    break;
				case 1:  invert = y % 2 == 0;                          break;
				case 2:  invert = x % 3 == 0;                          break;
				case 3:  invert = (x + y) % 3 == 0;                    break;
				case 4:  invert = (x / 3 + y / 2) % 2 == 0;            break;
				case 5:  invert = x * y % 2 + x * y % 3 == 0;          break;
				case 6:  invert = (x * y % 2 + x * y % 3) % 2 == 0;    break;
				case 7:  invert = ((x + y) % 2 + x * y % 3) % 2 == 0;  break;
				default:  throw std::logic_error("Unreachable");
			}
			modules[y][x] = modules[y][x] ^ (invert & !isFunction[y][x]);
		}
	}
}

long QrCode::getPenaltyScore() const {
	long result = 0;
	
	for (int y = 0; y < size; y++) {
		bool runColor = false;
		int runX = 0;
		std::array<int, 7> runHistory = {};
		for (int x = 0; x < size; x++) {
			if (module(x, y) == runColor) {
				runX++;
				if (runX == 5)
					result += 3;
				else if (runX > 5)
					result++;
			} else {
				finderPenaltyAddHistory(runX, runHistory);
				if (!runColor)
					result += finderPenaltyCountPatterns(runHistory) * 40;
				runColor = module(x, y);
				runX = 1;
			}
		}
		result += finderPenaltyTerminateAndCount(runColor, runX, runHistory) * 40;
	}
	for (int x = 0; x < size; x++) {
		bool runColor = false;
		int runY = 0;
		std::array<int, 7> runHistory = {};
		for (int y = 0; y < size; y++) {
			if (module(x, y) == runColor) {
				runY++;
				if (runY == 5)
					result += 3;
				else if (runY > 5)
					result++;
			} else {
				finderPenaltyAddHistory(runY, runHistory);
				if (!runColor)
					result += finderPenaltyCountPatterns(runHistory) * 40;
				runColor = module(x, y);
				runY = 1;
			}
		}
		result += finderPenaltyTerminateAndCount(runColor, runY, runHistory) * 40;
	}
	
	for (int y = 0; y < size - 1; y++) {
		for (int x = 0; x < size - 1; x++) {
			bool color = module(x, y);
			if (color == module(x + 1, y) &&
			    color == module(x, y + 1) &&
			    color == module(x + 1, y + 1))
				result += 3;
		}
	}
	
	int black = 0;
	for (const vector<bool> &row : modules) {
		for (bool color : row) {
			if (color)
				black++;
		}
	}
	int total = size * size;
	int k = static_cast<int>((std::abs(black * 20L - total * 10L) + total - 1) / total) - 1;
	result += k * 10;
	return result;
}

int QrCode::getAlignmentPatternPositions() const {
	if (version == 1)
		return 0;
	else {
		int numAlign = version / 7 + 2;
		return numAlign;
	}
}

int QrCode::getNumDataCodewords(int ver, Ecc ecl) {
	return getNumRawDataModules(ver) / 8 -
		ECC_CODEWORDS_PER_BLOCK    [static_cast<int>(ecl)][ver] *
		NUM_ERROR_CORRECTION_BLOCKS[static_cast<int>(ecl)][ver];
}

int QrCode::getNumRawDataModules(int ver) {
	if (ver < MIN_VERSION || ver > MAX_VERSION)
		throw std::domain_error("Version out of range");
	int result = (16 * ver + 128) * ver + 64;
	if (ver >= 2) {
		int numAlign = ver / 7 + 2;
		result -= (25 * numAlign - 10) * numAlign - 55;
		if (ver >= 7)
			result -= 36;
	}
	return result;
}

vector<uint8_t> QrCode::reedSolomonComputeDivisor(int degree) {
	if (degree < 1 || degree > 255)
		throw std::domain_error("Degree out of range");
	vector<uint8_t> result(static_cast<size_t>(degree));
	result.back() = 1;
	
	uint8_t root = 1;
	for (int i = 0; i < degree; i++) {
		for (size_t j = 0; j < result.size(); j++) {
			result[j] = reedSolomonMultiply(result[j], root);
			if (j + 1 < result.size())
				result[j] ^= result[j + 1];
		}
		root = reedSolomonMultiply(root, 0x02);
	}
	return result;
}

vector<uint8_t> QrCode::reedSolomonComputeRemainder(const vector<uint8_t> &data, const vector<uint8_t> &divisor) {
	vector<uint8_t> result(divisor.size());
	for (uint8_t b : data) {
		uint8_t factor = b ^ result[0];
		result.erase(result.begin());
		result.push_back(0);
		for (size_t i = 0; i < divisor.size(); i++)
			result[i] ^= reedSolomonMultiply(divisor[i], factor);
	}
	return result;
}

uint8_t QrCode::reedSolomonMultiply(uint8_t x, uint8_t y) {
	int z = 0;
	for (int i = 7; i >= 0; i--) {
		z = (z << 1) ^ ((z >> 7) * 0x11D);
		z ^= ((y >> i) & 1) * x;
	}
	assert(z >> 8 == 0);
	return static_cast<uint8_t>(z);
}

int QrCode::finderPenaltyCountPatterns(const std::array<int, 7> &runHistory) const {
	int n = runHistory[1];
	assert(n <= size * 3);
	bool core = n > 0 && runHistory[2] == n && runHistory[3] == n * 3 && runHistory[4] == n && runHistory[5] == n;
	return (core && runHistory[0] >= n * 4 && runHistory[6] >= n ? 1 : 0)
	     + (core && runHistory[6] >= n * 4 && runHistory[0] >= n ? 1 : 0);
}

int QrCode::finderPenaltyTerminateAndCount(bool currentRunColor, int currentRunLength, std::array<int, 7> &runHistory) const {
	if (currentRunColor) {
		finderPenaltyAddHistory(currentRunLength, runHistory);
		currentRunLength = 0;
	}
	currentRunLength += size;
	finderPenaltyAddHistory(currentRunLength, runHistory);
	return finderPenaltyCountPatterns(runHistory);
}

void QrCode::finderPenaltyAddHistory(int currentRunLength, std::array<int, 7> &runHistory) const {
	if (runHistory[0] == 0)
		currentRunLength += size;
	std::copy_backward(runHistory.cbegin(), runHistory.cend() - 1, runHistory.end());
	runHistory[0] = currentRunLength;
}

const int8_t QrCode::ECC_CODEWORDS_PER_BLOCK[4][41] = {
	{-1,  7, 10, 15, 20, 26, 18, 20, 24, 30, 18, 20, 24, 26, 30, 22, 24, 28, 30, 28, 28, 28, 28, 30, 30, 26, 28, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30},
	{-1, 10, 16, 26, 18, 24, 16, 18, 22, 22, 26, 30, 22, 22, 24, 24, 28, 28, 26, 26, 26, 26, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28},
	{-1, 13, 22, 18, 26, 18, 24, 18, 22, 20, 24, 28, 26, 24, 20, 30, 24, 28, 28, 26, 30, 28, 30, 30, 30, 30, 28, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30},
	{-1, 17, 28, 22, 16, 22, 28, 26, 26, 24, 28, 24, 28, 22, 24, 24, 30, 28, 28, 26, 28, 30, 24, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30},
};

const int8_t QrCode::NUM_ERROR_CORRECTION_BLOCKS[4][41] = {
	{-1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 4, 4, 4, 4, 4, 6, 6, 6, 6, 7, 8, 8,  9,  9, 10, 12, 12, 12, 13, 14, 15, 16, 17, 18, 19, 19, 20, 21, 22, 24, 25},
	{-1, 1, 1, 1, 2, 2, 4, 4, 4, 5, 5, 5, 8, 9, 9, 10, 10, 11, 13, 14, 16, 17, 17, 18, 20, 21, 23, 25, 26, 28, 29, 31, 33, 35, 37, 38, 40, 43, 45, 47, 49},
	{-1, 1, 1, 2, 2, 4, 4, 6, 6, 8, 8, 8, 10, 12, 16, 12, 17, 16, 18, 21, 20, 23, 23, 25, 27, 29, 34, 34, 35, 38, 40, 43, 45, 48, 51, 53, 56, 59, 62, 65, 68},
	{-1, 1, 1, 2, 4, 4, 4, 5, 6, 8, 8, 11, 11, 16, 16, 18, 16, 19, 21, 25, 25, 25, 34, 30, 32, 35, 37, 40, 42, 45, 48, 51, 54, 57, 60, 63, 66, 70, 74, 77, 81},
};

data_too_long::data_too_long(const std::string &msg) :
	std::length_error(msg) {}

BitBuffer::BitBuffer() :
	std::vector<bool>() {}

void BitBuffer::appendBits(std::uint32_t val, int len) {
	if (len < 0 || len > 31 || val >> len != 0)
		throw std::domain_error("Value out of range");
	for (int i = len - 1; i >= 0; i--)
		this->push_back(((val >> i) & 1) != 0);
}

}
