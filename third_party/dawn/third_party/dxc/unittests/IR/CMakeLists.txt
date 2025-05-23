set(LLVM_LINK_COMPONENTS
  Analysis
  AsmParser
  Core
  IPA
  Support
  )

set(IRSources
  AsmWriterTest.cpp
  AttributesTest.cpp
  ConstantRangeTest.cpp
  ConstantsTest.cpp
  DebugInfoTest.cpp
  DominatorTreeTest.cpp
  IRBuilderTest.cpp
  InstructionsTest.cpp
  LegacyPassManagerTest.cpp
  MDBuilderTest.cpp
  MetadataTest.cpp
  PassManagerTest.cpp
  PatternMatch.cpp
  TypeBuilderTest.cpp
  TypesTest.cpp
  UseTest.cpp
  UserTest.cpp
  ValueHandleTest.cpp
  ValueMapTest.cpp
  ValueTest.cpp
  VerifierTest.cpp
  WaymarkTest.cpp
  )

# HACK: Declare a couple of source files as optionally compiled to satisfy the
# missing-file-checker in LLVM's weird CMake build.
set(LLVM_OPTIONAL_SOURCES
  ValueMapTest.cpp
  )

add_llvm_unittest(IRTests
  ${IRSources}
  )
