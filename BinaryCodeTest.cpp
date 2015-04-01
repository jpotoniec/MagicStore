#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE BinaryCodeTest
#include <boost/test/unit_test.hpp>
#include "Compressor.hpp"

BOOST_AUTO_TEST_CASE(AlignmentTest)
{
	BinaryCode b(0b1011, 4);
	BOOST_CHECK_EQUAL(b.value(), 0b1011);
	BOOST_CHECK_EQUAL(b.leftNormal(), 0b10110000000000000000000000000000);
	BOOST_CHECK_EQUAL(b.rightReversed(), 0b1101);
}

BOOST_AUTO_TEST_CASE(MaskLeft)
{
	BinaryCode b(0, 4);
	BOOST_CHECK_EQUAL(b.maskLeft(), 0b11110000000000000000000000000000);
}

BOOST_AUTO_TEST_CASE(MaskRight)
{
	BinaryCode b(0, 4);
	BOOST_CHECK_EQUAL(b.maskRight(), 0b1111);
}

void write(uint8_t *where, size_t &wlen, const BinaryCode& c);
uint32_t read(const uint8_t *where, size_t &position);
BOOST_AUTO_TEST_CASE(Write)
{
	uint8_t *data=new uint8_t[128];
	memset(data, 0, 128);
	size_t len=0;
	write(data, len, BinaryCode(0b1011, 4));
	write(data, len, BinaryCode(0b110010100011, 12));
	BOOST_CHECK_EQUAL(len, 3);
	size_t pos=0;
	BOOST_CHECK_EQUAL(read(data, pos), 0b1011);
	BOOST_CHECK_EQUAL(read(data, pos), 0b110010100011);
	BOOST_CHECK_EQUAL(pos, len);
}
