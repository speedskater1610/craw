module Lexer where

import Data.Char
import Data.List
import AST

data Token
  = TokIdent String
  | TokNumber Integer
  | TokString String
  | TokComma
  | TokColon
  | TokLBrack
  | TokRBrack
  | TokPlus
  | TokMinus
  | TokStar
  | TokSlash
  | TokPercent
  | TokDollar
  | TokNewline
  | TokEOF
  | TokKeyword String
  deriving (Show, Eq)

data LexState = LexState
  { lexInput :: String
  , lexFile :: String
  , lexLine :: Int
  , lexCol :: Int
  }

tokenize :: String -> String -> Either AsmError [Token]
tokenize filename input = 
  let state = LexState (stripComments input) filename 1 1
  in lexTokens state []

stripComments :: String -> String
stripComments [] = []
stripComments (';':rest) = '\n' : stripComments (dropWhile (/= '\n') rest)
stripComments (c:cs) = c : stripComments cs

lexTokens :: LexState -> [Token] -> Either AsmError [Token]
lexTokens state acc
  | null (lexInput state) = Right (reverse (TokEOF : acc))
  | otherwise = 
      case lexOne state of
        Left err -> Left err
        Right (Nothing, state') -> lexTokens state' acc
        Right (Just tok, state') -> lexTokens state' (tok : acc)

lexOne :: LexState -> Either AsmError (Maybe Token, LexState)
lexOne state@(LexState [] _ _ _) = Right (Nothing, state)
lexOne state@(LexState (c:cs) file line col)
  | isSpace c && c /= '\n' = 
      Right (Nothing, LexState cs file line (col + 1))
  | c == '\n' = 
      Right (Just TokNewline, LexState cs file (line + 1) 1)
  | c == ',' = Right (Just TokComma, advance state)
  | c == ':' = Right (Just TokColon, advance state)
  | c == '[' = Right (Just TokLBrack, advance state)
  | c == ']' = Right (Just TokRBrack, advance state)
  | c == '+' = Right (Just TokPlus, advance state)
  | c == '-' = Right (Just TokMinus, advance state)
  | c == '*' = Right (Just TokStar, advance state)
  | c == '/' = Right (Just TokSlash, advance state)
  | c == '$' = Right (Just TokDollar, advance state)
  | c == '%' = lexDirectiveOrReg state
  | c == '\'' || c == '"' = lexString c state
  | isDigit c = lexNumber state
  | isAlpha c || c == '_' || c == '.' = lexIdentifier state
  | otherwise = Left $ AsmError (SourceLoc file line col) 
                  ("Unexpected character: " ++ [c])

advance :: LexState -> LexState
advance (LexState (_:cs) f l c) = LexState cs f l (c + 1)
advance state = state

lexDirectiveOrReg :: LexState -> Either AsmError (Maybe Token, LexState)
lexDirectiveOrReg state@(LexState input file line col) =
  let (word, rest) = span isAlphaNum (tail input)
      newCol = col + 1 + length word
  in if null word
     then Right (Just TokPercent, advance state)
     else Right (Just (TokKeyword ('%':word)), 
                LexState rest file line newCol)

lexString :: Char -> LexState -> Either AsmError (Maybe Token, LexState)
lexString quote (LexState (_:cs) file line col) =
  case readString cs [] of
    Nothing -> Left $ AsmError (SourceLoc file line col) "Unterminated string"
    Just (str, rest) -> 
      Right (Just (TokString str), 
             LexState rest file line (col + length str + 2))
  where
    readString [] _ = Nothing
    readString (c:cs) acc
      | c == quote = Just (reverse acc, cs)
      | c == '\\' && not (null cs) = readString (tail cs) (head cs : acc)
      | otherwise = readString cs (c : acc)

lexNumber :: LexState -> Either AsmError (Maybe Token, LexState)
lexNumber state@(LexState input file line col) =
  let (numStr, rest) = span isAlphaNum input
      num = if "0x" `isPrefixOf` numStr || "0X" `isPrefixOf` numStr
            then read ("0x" ++ drop 2 numStr) :: Integer
            else if last numStr == 'h' || last numStr == 'H'
            then read ("0x" ++ init numStr) :: Integer
            else read numStr :: Integer
  in Right (Just (TokNumber num), 
           LexState rest file line (col + length numStr))

lexIdentifier :: LexState -> Either AsmError (Maybe Token, LexState)
lexIdentifier (LexState input file line col) =
  let (ident, rest) = span (\c -> isAlphaNum c || c == '_' || c == '.') input
      tok = if ident `elem` keywords
            then TokKeyword ident
            else TokIdent ident
  in Right (Just tok, LexState rest file line (col + length ident))

keywords :: [String]
keywords = 
  [ "byte", "word", "dword", "qword"
  , "db", "dw", "dd", "dq"
  , "resb", "resw", "resd", "resq"
  , "equ", "align", "section"
  , "global", "extern"
  , "short", "near", "far"
  ]
