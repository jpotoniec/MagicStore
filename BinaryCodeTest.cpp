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
