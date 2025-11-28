module Parser where

import AST
import Lexer
import Data.List
import Data.Char (toUpper)

data ParserState = ParserState
  { parserTokens :: [Token]
  , parserFile :: String
  , parserLine :: Int
  } deriving (Show)

parse :: String -> [Token] -> Either AsmError Program
parse filename tokens = 
  let state = ParserState tokens filename 1
  in do
    stmts <- parseStatements state []
    return $ Program stmts filename

parseStatements :: ParserState -> [Statement] -> Either AsmError [Statement]
parseStatements state acc
  | null (parserTokens state) = Right (reverse acc)
  | otherwise = do
      (stmt, state') <- parseStatement state
      case stmt of
        Nothing -> parseStatements state' acc
        Just s -> parseStatements state' (s : acc)

parseStatement :: ParserState -> Either AsmError (Maybe Statement, ParserState)
parseStatement state = 
  case parserTokens state of
    [] -> Right (Nothing, state)
    (TokNewline:rest) -> 
      Right (Nothing, state { parserTokens = rest, parserLine = parserLine state + 1 })
    (TokEOF:_) -> Right (Nothing, state)
    (TokIdent label : TokColon : rest) -> do
      let loc = SourceLoc (parserFile state) (parserLine state) 1
          newState = state { parserTokens = rest }
      Right (Just (StmtLabel label loc), newState)
    (TokKeyword kw : rest)
      | kw `elem` ["db", "dw", "dd", "dq", "resb", "resw", "resd", "resq", 
                   "equ", "align", "section", "global", "extern"] -> 
          parseDirective state
    tokens -> parseInstruction state

parseDirective :: ParserState -> Either AsmError (Maybe Statement, ParserState)
parseDirective state = 
  case parserTokens state of
    (TokKeyword "section" : TokIdent name : rest) -> do
      let loc = SourceLoc (parserFile state) (parserLine state) 1
          dir = DirSection name
      state' <- consumeNewline (state { parserTokens = rest })
      Right (Just (StmtDir dir loc), state')
    
    (TokKeyword "section" : TokKeyword name : rest) -> do
      let loc = SourceLoc (parserFile state) (parserLine state) 1
          dir = DirSection name
      state' <- consumeNewline (state { parserTokens = rest })
      Right (Just (StmtDir dir loc), state')
    
    (TokKeyword "global" : TokIdent name : rest) -> do
      let loc = SourceLoc (parserFile state) (parserLine state) 1
          dir = DirGlobal name
      state' <- consumeNewline (state { parserTokens = rest })
      Right (Just (StmtDir dir loc), state')
    
    (TokKeyword "extern" : TokIdent name : rest) -> do
      let loc = SourceLoc (parserFile state) (parserLine state) 1
          dir = DirExtern name
      state' <- consumeNewline (state { parserTokens = rest })
      Right (Just (StmtDir dir loc), state')
    
    (TokIdent label : TokKeyword "equ" : rest) -> do
      (expr, state') <- parseExpression (state { parserTokens = rest })
      let loc = SourceLoc (parserFile state) (parserLine state) 1
          dir = DirEqu label expr
      state'' <- consumeNewline state'
      Right (Just (StmtDir dir loc), state'')
    
    (TokKeyword "align" : TokNumber n : rest) -> do
      let loc = SourceLoc (parserFile state) (parserLine state) 1
          dir = DirAlign (NumLit (fromIntegral n))
      state' <- consumeNewline (state { parserTokens = rest })
      Right (Just (StmtDir dir loc), state')
    
    (TokKeyword dtype : rest)
      | dtype `elem` ["db", "dw", "dd", "dq"] -> do
          (exprs, state') <- parseExprList (state { parserTokens = rest }) []
          let loc = SourceLoc (parserFile state) (parserLine state) 1
              dir = case dtype of
                "db" -> DirDB exprs
                "dw" -> DirDW exprs
                "dd" -> DirDD exprs
                "dq" -> DirDD exprs
          state'' <- consumeNewline state'
          Right (Just (StmtDir dir loc), state'')
    
    (TokKeyword rtype : TokNumber count : rest)
      | rtype `elem` ["resb", "resw", "resd", "resq"] -> do
          let loc = SourceLoc (parserFile state) (parserLine state) 1
              expr = NumLit (fromIntegral count)
              dir = case rtype of
                "resb" -> DirResB expr
                "resw" -> DirResW expr
                "resd" -> DirResD expr
                "resq" -> DirResD expr
          state' <- consumeNewline (state { parserTokens = rest })
          Right (Just (StmtDir dir loc), state')
    
    _ -> Left $ AsmError 
      (SourceLoc (parserFile state) (parserLine state) 1)
      "Invalid directive"

parseInstruction :: ParserState -> Either AsmError (Maybe Statement, ParserState)
parseInstruction state = 
  case parserTokens state of
    (TokIdent mnemonic : rest) -> do
      let state' = state { parserTokens = rest }
      (ops, state'') <- parseOperands state' []
      let loc = SourceLoc (parserFile state) (parserLine state) 1
          instr = Instruction mnemonic ops loc
      state''' <- consumeNewline state''
      Right (Just (StmtInstr instr), state''')
    _ -> Left $ AsmError 
      (SourceLoc (parserFile state) (parserLine state) 1)
      "Expected instruction mnemonic"

parseOperands :: ParserState -> [Operand] -> Either AsmError ([Operand], ParserState)
parseOperands state acc = 
  case parserTokens state of
    (TokNewline:_) -> Right (reverse acc, state)
    (TokEOF:_) -> Right (reverse acc, state)
    [] -> Right (reverse acc, state)
    tokens -> do
      (op, state') <- parseOperand state
      case parserTokens state' of
        (TokComma : rest) -> 
          parseOperands (state' { parserTokens = rest }) (op : acc)
        _ -> Right (reverse (op : acc), state')

parseOperand :: ParserState -> Either AsmError (Operand, ParserState)
parseOperand state = 
  case parserTokens state of
    (TokIdent reg : rest)
      | reg `elem` registerNames -> 
          Right (OpReg (parseRegister reg), state { parserTokens = rest })
    
    (TokKeyword size : TokLBrack : rest)
      | size `elem` ["byte", "word", "dword", "qword"] -> do
          let opSize = case size of
                "byte" -> Byte
                "word" -> Word
                "dword" -> Dword
                "qword" -> Qword
          parseMemory (state { parserTokens = rest }) opSize
    
    (TokLBrack : rest) -> 
      parseMemory (state { parserTokens = rest }) UnknownSize
    
    tokens -> do
      (expr, state') <- parseExpression state
      Right (OpImm expr, state')

parseMemory :: ParserState -> OpSize -> Either AsmError (Operand, ParserState)
parseMemory state size = do
  (base, index, scale, disp, state') <- parseMemoryExpr state Nothing Nothing 1 (NumLit 0)
  case parserTokens state' of
    (TokRBrack : rest) -> 
      let mem = Memory size base index scale disp
      in Right (OpMem mem, state' { parserTokens = rest })
    _ -> Left $ AsmError 
      (SourceLoc (parserFile state) (parserLine state) 1)
      "Expected ']'"

parseMemoryExpr :: ParserState -> Maybe Register -> Maybe Register -> Int -> Expr
                -> Either AsmError (Maybe Register, Maybe Register, Int, Expr, ParserState)
parseMemoryExpr state base index scale disp = 
  case parserTokens state of
    (TokRBrack:_) -> Right (base, index, scale, disp, state)
    (TokIdent reg : rest)
      | reg `elem` registerNames -> 
          let r = parseRegister reg
              state' = state { parserTokens = rest }
          in case parserTokens state' of
            (TokStar : TokNumber n : rest') -> 
              parseMemoryExpr (state' { parserTokens = rest' }) base (Just r) (fromIntegral n) disp
            (TokPlus : rest') -> 
              if base == Nothing
                then parseMemoryExpr (state' { parserTokens = rest' }) (Just r) index scale disp
                else parseMemoryExpr (state' { parserTokens = rest' }) base (Just r) scale disp
            _ -> parseMemoryExpr state' (Just r) index scale disp
    
    (TokNumber n : rest) -> 
      let newDisp = BinOp Add disp (NumLit (fromIntegral n))
          state' = state { parserTokens = rest }
      in case parserTokens state' of
        (TokPlus : rest') -> parseMemoryExpr (state' { parserTokens = rest' }) base index scale newDisp
        _ -> parseMemoryExpr state' base index scale newDisp
    
    (TokPlus : rest) -> parseMemoryExpr (state { parserTokens = rest }) base index scale disp
    (TokMinus : TokNumber n : rest) -> 
      let newDisp = BinOp Sub disp (NumLit (fromIntegral n))
      in parseMemoryExpr (state { parserTokens = rest }) base index scale newDisp
    
    _ -> Right (base, index, scale, disp, state)

parseExpression :: ParserState -> Either AsmError (Expr, ParserState)
parseExpression state = parseAddSub state

parseAddSub :: ParserState -> Either AsmError (Expr, ParserState)
parseAddSub state = do
  (left, state') <- parseTerm state
  case parserTokens state' of
    (TokPlus : rest) -> do
      (right, state'') <- parseAddSub (state' { parserTokens = rest })
      Right (BinOp Add left right, state'')
    (TokMinus : rest) -> do
      (right, state'') <- parseAddSub (state' { parserTokens = rest })
      Right (BinOp Sub left right, state'')
    _ -> Right (left, state')

parseTerm :: ParserState -> Either AsmError (Expr, ParserState)
parseTerm state = 
  case parserTokens state of
    (TokNumber n : rest) -> 
      Right (NumLit (fromIntegral n), state { parserTokens = rest })
    (TokIdent name : rest) -> 
      Right (Ident name, state { parserTokens = rest })
    (TokDollar : rest) ->
      Right (Ident "$", state { parserTokens = rest })
    (TokMinus : rest) -> do
      (expr, state') <- parseTerm (state { parserTokens = rest })
      Right (UnOp Neg expr, state')
    _ -> Left $ AsmError 
      (SourceLoc (parserFile state) (parserLine state) 1)
      "Expected expression"

parseExprList :: ParserState -> [Expr] -> Either AsmError ([Expr], ParserState)
parseExprList state acc = 
  case parserTokens state of
    (TokNewline:_) -> Right (reverse acc, state)
    (TokEOF:_) -> Right (reverse acc, state)
    [] -> Right (reverse acc, state)
    (TokString s : rest) -> do
      let exprs = map (NumLit . fromIntegral . fromEnum) s
          state' = state { parserTokens = rest }
      case parserTokens state' of
        (TokComma : rest') -> parseExprList (state' { parserTokens = rest' }) (exprs ++ acc)
        _ -> Right (reverse (exprs ++ acc), state')
    tokens -> do
      (expr, state') <- parseExpression state
      case parserTokens state' of
        (TokComma : rest) -> parseExprList (state' { parserTokens = rest }) (expr : acc)
        _ -> Right (reverse (expr : acc), state')

consumeNewline :: ParserState -> Either AsmError ParserState
consumeNewline state = 
  case parserTokens state of
    (TokNewline : rest) -> Right $ state { parserTokens = rest, parserLine = parserLine state + 1 }
    (TokEOF : _) -> Right state
    [] -> Right state
    _ -> Left $ AsmError 
      (SourceLoc (parserFile state) (parserLine state) 1)
      "Expected newline"

parseRegister :: String -> Register
parseRegister s = case map toUpper s of
  "AL" -> AL; "AH" -> AH; "AX" -> AX; "EAX" -> EAX
  "BL" -> BL; "BH" -> BH; "BX" -> BX; "EBX" -> EBX
  "CL" -> CL; "CH" -> CH; "CX" -> CX; "ECX" -> ECX
  "DL" -> DL; "DH" -> DH; "DX" -> DX; "EDX" -> EDX
  "SI" -> SI; "ESI" -> ESI; "DI" -> DI; "EDI" -> EDI
  "BP" -> BP; "EBP" -> EBP; "SP" -> SP; "ESP" -> ESP
  "CS" -> CS; "DS" -> DS; "ES" -> ES
  "FS" -> FS; "GS" -> GS; "SS" -> SS
  _ -> EAX

registerNames :: [String]
registerNames = 
  [ "al", "ah", "ax", "eax"
  , "bl", "bh", "bx", "ebx"
  , "cl", "ch", "cx", "ecx"
  , "dl", "dh", "dx", "edx"
  , "si", "esi", "di", "edi"
  , "bp", "ebp", "sp", "esp"
  , "cs", "ds", "es", "fs", "gs", "ss"
  ]
