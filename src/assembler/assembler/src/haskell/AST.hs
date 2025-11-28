{-# LANGUAGE DeriveGeneric #-}
{-# LANGUAGE ForeignFunctionInterface #-}

module AST where

import GHC.Generics
import Data.Word
import Data.Int

-- Source location for error reporting
data SourceLoc = SourceLoc
  { srcFile :: String
  , srcLine :: Int
  , srcCol :: Int
  } deriving (Show, Eq, Generic)

-- Register types
data Register 
  = AL | AH | AX | EAX
  | BL | BH | BX | EBX
  | CL | CH | CX | ECX
  | DL | DH | DX | EDX
  | SI | ESI | DI | EDI
  | BP | EBP | SP | ESP
  | CS | DS | ES | FS | GS | SS
  deriving (Show, Eq, Generic, Enum)

-- Operand sizes
data OpSize = Byte | Word | Dword | Qword | UnknownSize
  deriving (Show, Eq, Generic, Enum)

-- Expression for constants and addresses
data Expr
  = NumLit Int64
  | Ident String
  | BinOp BinOpType Expr Expr
  | UnOp UnOpType Expr
  deriving (Show, Eq, Generic)

data BinOpType = Add | Sub | Mul | Div | Mod | Shl | Shr
  deriving (Show, Eq, Generic, Enum)

data UnOpType = Neg | Not
  deriving (Show, Eq, Generic, Enum)

-- Memory addressing
data Memory = Memory
  { memSize :: OpSize
  , memBase :: Maybe Register
  , memIndex :: Maybe Register
  , memScale :: Int
  , memDisp :: Expr
  } deriving (Show, Eq, Generic)

-- Operand
data Operand
  = OpReg Register
  | OpMem Memory
  | OpImm Expr
  deriving (Show, Eq, Generic)

-- Instruction
data Instruction = Instruction
  { instrName :: String
  , instrOps :: [Operand]
  , instrLoc :: SourceLoc
  } deriving (Show, Eq, Generic)

-- Directives
data Directive
  = DirDB [Expr]         -- db 1, 2, 3
  | DirDW [Expr]         -- dw 100, 200
  | DirDD [Expr]         -- dd 12345
  | DirResB Expr         -- resb 10
  | DirResW Expr         -- resw 5
  | DirResD Expr         -- resd 3
  | DirEqu String Expr   -- label equ value
  | DirAlign Expr        -- align 16
  | DirSection String    -- section .text
  | DirGlobal String     -- global _start
  | DirExtern String     -- extern printf
  | DirInclude String    -- %include "file.inc"
  deriving (Show, Eq, Generic)

-- Statement
data Statement
  = StmtLabel String SourceLoc
  | StmtInstr Instruction
  | StmtDir Directive SourceLoc
  deriving (Show, Eq, Generic)

-- Complete program
data Program = Program
  { progStatements :: [Statement]
  , progFile :: String
  } deriving (Show, Eq, Generic)

-- Error reporting
data AsmError = AsmError
  { errLoc :: SourceLoc
  , errMsg :: String
  } deriving (Eq)

instance Show AsmError where
  show (AsmError loc msg) = 
    srcFile loc ++ ":" ++ show (srcLine loc) ++ ":" ++ 
    show (srcCol loc) ++ ": error: " ++ msg
