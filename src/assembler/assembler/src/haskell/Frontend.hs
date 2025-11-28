{-# LANGUAGE ForeignFunctionInterface #-}

module Frontend where

import AST
import Lexer
import Parser
import Preprocessor
import Foreign.C.String
import Foreign.C.Types
import Foreign.Ptr
import Foreign.Marshal.Array
import Foreign.Marshal.Alloc
import Foreign.Storable
import System.IO
import Data.Word
import Data.Int

-- FFI function to call Rust backend
foreign import ccall "assemble_to_object"
  c_assemble_to_object :: CString -> Ptr Word8 -> CInt -> CString -> IO CInt

compileFile :: String -> String -> IO (Either String ())
compileFile inputPath outputPath = do
  -- Read input file
  content <- readFile inputPath
  
  -- Preprocess
  ppResult <- preprocess inputPath content
  case ppResult of
    Left err -> return $ Left (show err)
    Right preprocessed -> do
      -- Tokenize
      let tokResult = tokenize inputPath preprocessed
      case tokResult of
        Left err -> return $ Left (show err)
        Right tokens -> do
          -- Parse
          let parseResult = parse inputPath tokens
          case parseResult of
            Left err -> return $ Left (show err)
            Right program -> do
              -- Serialize program to bytes and call Rust backend
              serializeAndAssemble program outputPath

serializeAndAssemble :: Program -> String -> IO (Either String ())
serializeAndAssemble program outputPath = do
  -- Serialize program to a simple binary format
  let bytes = serializeProgram program
  
  -- Call Rust backend
  withCString outputPath $ \cOutputPath -> do
    withArray bytes $ \bytesPtr -> do
      result <- c_assemble_to_object 
        nullPtr 
        bytesPtr 
        (fromIntegral $ length bytes) 
        cOutputPath
      
      if result == 0
        then return $ Right ()
        else return $ Left "Backend assembly failed"

-- Simple serialization format
serializeProgram :: Program -> [Word8]
serializeProgram (Program stmts _) = 
  concatMap serializeStatement stmts

serializeStatement :: Statement -> [Word8]
serializeStatement (StmtLabel name _) = 
  1 : (fromIntegral (length name) : map (fromIntegral . fromEnum) name)

serializeStatement (StmtInstr (Instruction mnemonic ops _)) = 
  2 : (fromIntegral (length mnemonic) : map (fromIntegral . fromEnum) mnemonic) ++
  [fromIntegral (length ops)] ++
  concatMap serializeOperand ops

serializeStatement (StmtDir dir _) = 
  serializeDirective dir

serializeOperand :: Operand -> [Word8]
serializeOperand (OpReg reg) = 
  [1, fromIntegral (fromEnum reg)]

serializeOperand (OpImm (NumLit n)) = 
  [2] ++ serializeInt64 n

serializeOperand (OpMem (Memory size base index scale disp)) = 
  [3, fromIntegral (fromEnum size)] ++
  serializeMaybeReg base ++
  serializeMaybeReg index ++
  [fromIntegral scale] ++
  serializeExpr disp

serializeOperand _ = [0]

serializeMaybeReg :: Maybe Register -> [Word8]
serializeMaybeReg Nothing = [0]
serializeMaybeReg (Just reg) = [1, fromIntegral (fromEnum reg)]

serializeExpr :: Expr -> [Word8]
serializeExpr (NumLit n) = [1] ++ serializeInt64 n
serializeExpr (Ident s) = [2, fromIntegral (length s)] ++ 
  map (fromIntegral . fromEnum) s
serializeExpr (BinOp op e1 e2) = 
  [3, fromIntegral (fromEnum op)] ++ serializeExpr e1 ++ serializeExpr e2
serializeExpr (UnOp op e) = 
  [4, fromIntegral (fromEnum op)] ++ serializeExpr e

serializeDirective :: Directive -> [Word8]
serializeDirective (DirSection name) = 
  [10, fromIntegral (length name)] ++ map (fromIntegral . fromEnum) name

serializeDirective (DirGlobal name) = 
  [11, fromIntegral (length name)] ++ map (fromIntegral . fromEnum) name

serializeDirective (DirExtern name) = 
  [12, fromIntegral (length name)] ++ map (fromIntegral . fromEnum) name

serializeDirective (DirDB exprs) = 
  [13, fromIntegral (length exprs)] ++ concatMap serializeExpr exprs

serializeDirective (DirDW exprs) = 
  [14, fromIntegral (length exprs)] ++ concatMap serializeExpr exprs

serializeDirective (DirDD exprs) = 
  [15, fromIntegral (length exprs)] ++ concatMap serializeExpr exprs

serializeDirective (DirResB expr) = 
  [16] ++ serializeExpr expr

serializeDirective (DirResW expr) = 
  [17] ++ serializeExpr expr

serializeDirective (DirResD expr) = 
  [18] ++ serializeExpr expr

serializeDirective (DirAlign expr) = 
  [19] ++ serializeExpr expr

serializeDirective _ = [0]

serializeInt64 :: Int64 -> [Word8]
serializeInt64 n = 
  [ fromIntegral (n `shiftR` 0)
  , fromIntegral (n `shiftR` 8)
  , fromIntegral (n `shiftR` 16)
  , fromIntegral (n `shiftR` 24)
  , fromIntegral (n `shiftR` 32)
  , fromIntegral (n `shiftR` 40)
  , fromIntegral (n `shiftR` 48)
  , fromIntegral (n `shiftR` 56)
  ]
  where
    shiftR = \x y -> fromIntegral ((fromIntegral x :: Word64) `div` (2 ^ y))
