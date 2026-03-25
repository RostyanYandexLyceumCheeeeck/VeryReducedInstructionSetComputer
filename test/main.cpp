#include "headers/emulator.h"
#include <gtest/gtest.h>
#include <string>


class VriscTest : public ::testing::Test {
protected:
    int run(const std::string& program) {
        return Emulator::emulate(program);
    }
};


TEST_F(VriscTest, MovImmediate_SetsRegister) {
    EXPECT_EQ(run(R"(Mov R0 42)"), 42);
}

TEST_F(VriscTest, MovRegister_CopiesValue) {
    EXPECT_EQ(run(R"(
        Mov R1 100
        Mov R0 R1
    )"), 100);
}

TEST_F(VriscTest, AddImmediate_AddsConstant) {
    EXPECT_EQ(run(R"(
        Mov R0 10
        Add R0 5
    )"), 15);
}

TEST_F(VriscTest, AddRegister_AddsRegister) {
    EXPECT_EQ(run(R"(
        Mov R0 7
        Mov R1 3
        Add R0 R1
    )"), 10);
}

TEST_F(VriscTest, SubImmediate_SubtractsConstant) {
    EXPECT_EQ(run(R"(
        Mov R0 20
        Sub R0 8
    )"), 12);
}

TEST_F(VriscTest, SubRegister_SubtractsRegister) {
    EXPECT_EQ(run(R"(
        Mov R0 50
        Mov R1 15
        Sub R0 R1
    )"), 35);
}

TEST_F(VriscTest, MulImmediate_MultipliesConstant) {
    EXPECT_EQ(run(R"(
        Mov R0 6
        Mul R0 7
    )"), 42);
}

TEST_F(VriscTest, MulRegister_MultipliesRegister) {
    EXPECT_EQ(run(R"(
        Mov R0 4
        Mov R1 5
        Mul R0 R1
    )"), 20);
}

TEST_F(VriscTest, DivImmediate_DividesConstant) {
    EXPECT_EQ(run(R"(
        Mov R0 100
        Div R0 4
    )"), 25);
}

TEST_F(VriscTest, DivRegister_DividesRegister) {
    EXPECT_EQ(run(R"(
        Mov R0 81
        Mov R1 9
        Div R0 R1
    )"), 9);
}

TEST_F(VriscTest, DivTruncates_IntegerDivision) {
    EXPECT_EQ(run(R"(
        Mov R0 7
        Div R0 2
    )"), 3);
}

TEST_F(VriscTest, StoreThenLoad_RoundTrip) {
    EXPECT_EQ(run(R"(
        Mov R0 123
        Store R0 50      

        Mov R0 0         
        Load R0 50       
    )"), 123);
}

TEST_F(VriscTest, MemoryMultipleCells_Independent) {
    EXPECT_EQ(run(R"(
        Mov R0 10
        Store R0 100     

        Mov R1 5
        Store R1 101     

        Load R0 100      
        Load R1 101     

        Add R0 R1        
    )"), 15);
}

TEST_F(VriscTest, Jmp_UnconditionalJump) {
    EXPECT_EQ(run(R"(
        Mov R1 42
        Jmp 3
        Mov R1 999
        Mov R0 R1
    )"), 42);
}

TEST_F(VriscTest, Jmpz_Taken_WhenR0IsZero) {
    EXPECT_EQ(run(R"(
        Mov R0 0
        Jmpz 4
        Mov R0 999
        Mov R0 998
        Mov R0 77
    )"), 77);
}

TEST_F(VriscTest, Jmpz_NotTaken_WhenR0NonZero) {
    EXPECT_EQ(run(R"(
        Mov R0 1
        Jmpz 3
        Add R0 10        
        Jmp 6
        Mov R0 999
        Mov R0 998
    )"), 11);
}

TEST_F(VriscTest, Jmpz_AfterModification) {
    EXPECT_EQ(run(R"(
        Mov R0 5
        Sub R0 5         
        Jmpz 4           
        Mov R0 999       
        Mov R0 42
    )"), 42);
}

TEST_F(VriscTest, Factorial_5) {
    EXPECT_EQ(run(R"(
        Mov R0 5
        Mov R1 1    
        Jmpz 6  
                    
        Mul R1 R0   
        Sub R0 1    
        Jmp 2       

        Mov R0 R1
    )"), 120);
}

TEST_F(VriscTest, ArifmeticProgression_1_to_10) {
    EXPECT_EQ(run(R"(
        Mov R0 10        
        Mov R1 0         
    
        Jmpz 6           
        Add R1 R0        
        Sub R0 1         
        Jmp 2            
        Mov R0 R1        
    )"), 55);
}

TEST_F(VriscTest, Power_2_to_4) {
    EXPECT_EQ(run(R"(
        Mov R0 4         
        Mov R1 2         
        Mov R2 1         
    
        Jmpz 7           
        Mul R2 R1        
        Sub R0 1         
        Jmp 3            
        Mov R0 R2        
    )"), 16);
}

TEST_F(VriscTest, NegativeResult) {
    EXPECT_EQ(run(R"(
        Mov R0 5
        Sub R0 10        
    )"), -5);
}

TEST_F(VriscTest, ChainOfMov) {
    EXPECT_EQ(run(R"(
        Mov R0 1
        Mov R1 R0
        Mov R2 R1
        Mov R3 R2
        Mov R0 R3
    )"), 1);
}

TEST_F(VriscTest, ComplexExpression) {
    EXPECT_EQ(run(R"(
        Mov R0 10
        Add R0 5         
        Mul R0 2         
        Sub R0 4         
    )"), 26);
}

TEST_F(VriscTest, UnknownInstruction) {
    EXPECT_THROW(
        run(R"(asd R0 1)"),
        std::invalid_argument
    );
}

TEST_F(VriscTest, InvalidRegister) {
    EXPECT_THROW(
        run(R"(Mov R9 10)"),
        std::invalid_argument
    );
}

TEST_F(VriscTest, InvalidArguments) {
    EXPECT_THROW(
        run(R"(Add R0)"),
        std::invalid_argument
    );
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}