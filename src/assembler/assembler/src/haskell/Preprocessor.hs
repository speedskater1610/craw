module Preprocessor where

import AST
import Lexer
import Data.List
import Data.Maybe
import qualified Data.Map.Strict as M
import System.IO
import System.Directory

data PreprocessorState = PreprocessorState
  { ppDefines :: M.Map String Integer
  , ppCondStack :: [Bool]  -- Track %if nesting
  , ppIncludeDirs :: [String]
  , ppCurrentFile :: String
  } deriving (Show)

initialPPState :: String -> PreprocessorState
initialPPState file = PreprocessorState
  { ppDefines = M.empty
  , ppCondStack = []
  , ppIncludeDirs = [".", "/usr/include", "/usr/local/include"]
  , ppCurrentFile = file
  }

preprocess :: String -> String -> IO (Either AsmError String)
preprocess filename content = do
  let state = initialPPState filename
  processLines state (lines content) []

processLines :: PreprocessorState -> [String] -> [String] 
             -> IO (Either AsmError String)
processLines state [] acc = 
  return $ Right $ unlines (reverse acc)
processLines state (line:rest) acc
  | "%include" `isPrefixOf` trimmed = 
      handleInclude state rest acc (words trimmed)
  | "%define" `isPrefixOf` trimmed = 
      handleDefine state rest acc (words trimmed)
  | "%assign" `isPrefixOf` trimmed = 
      handleAssign state rest acc (words trimmed)
  | "%if" `isPrefixOf` trimmed = 
      handleIf state rest acc (drop 3 trimmed)
  | "%ifdef" `isPrefixOf` trimmed = 
      handleIfdef state rest acc (words trimmed)
  | "%ifndef" `isPrefixOf` trimmed = 
      handleIfndef state rest acc (words trimmed)
  | "%elif" `isPrefixOf` trimmed = 
      handleElif state rest acc
  | "%else" `isPrefixOf` trimmed = 
      handleElse state rest acc
  | "%endif" `isPrefixOf` trimmed = 
      handleEndif state rest acc
  | "%rep" `isPrefixOf` trimmed = 
      handleRep state rest acc (drop 4 trimmed)
  | shouldIncludeLine state = 
      processLines state rest (expandDefines state line : acc)
  | otherwise = 
      processLines state rest acc
  where
    trimmed = dropWhile (== ' ') line

shouldIncludeLine :: PreprocessorState -> Bool
shouldIncludeLine state = 
  null (ppCondStack state) || and (ppCondStack state)

expandDefines :: PreprocessorState -> String -> String
expandDefines state line = 
  foldl' replaceDefine line (M.toList (ppDefines state))
  where
    replaceDefine str (name, val) = 
      replace name (show val) str

replace :: String -> String -> String -> String
replace [] _ str = str
replace _ _ [] = []
replace old new str@(c:cs)
  | old `isPrefixOf` str = new ++ replace old new (drop (length old) str)
  | otherwise = c : replace old new cs

handleInclude :: PreprocessorState -> [String] -> [String] -> [String]
              -> IO (Either AsmError String)
handleInclude state rest acc tokens = do
  case tokens of
    [_, filename] -> do
      let cleanName = filter (`notElem` "\"'") filename
      found <- findIncludeFile (ppIncludeDirs state) cleanName
      case found of
        Nothing -> return $ Left $ AsmError 
          (SourceLoc (ppCurrentFile state) 0 0)
          ("Cannot find include file: " ++ cleanName)
        Just path -> do
          content <- readFile path
          result <- preprocess path content
          case result of
            Left err -> return $ Left err
            Right included -> processLines state rest (included : acc)
    _ -> return $ Left $ AsmError 
      (SourceLoc (ppCurrentFile state) 0 0)
      "Invalid %include directive"

findIncludeFile :: [String] -> String -> IO (Maybe String)
findIncludeFile [] _ = return Nothing
findIncludeFile (dir:dirs) filename = do
  let path = dir ++ "/" ++ filename
  exists <- doesFileExist path
  if exists
    then return (Just path)
    else findIncludeFile dirs filename

handleDefine :: PreprocessorState -> [String] -> [String] -> [String]
             -> IO (Either AsmError String)
handleDefine state rest acc tokens = 
  case tokens of
    [_, name, value] -> 
      let val = read value :: Integer
          newState = state { ppDefines = M.insert name val (ppDefines state) }
      in processLines newState rest acc
    _ -> return $ Left $ AsmError 
      (SourceLoc (ppCurrentFile state) 0 0)
      "Invalid %define directive"

handleAssign :: PreprocessorState -> [String] -> [String] -> [String]
             -> IO (Either AsmError String)
handleAssign = handleDefine

handleIf :: PreprocessorState -> [String] -> [String] -> String
         -> IO (Either AsmError String)
handleIf state rest acc condition = 
  let cond = evaluateCondition state condition
      newState = state { ppCondStack = cond : ppCondStack state }
  in processLines newState rest acc

handleIfdef :: PreprocessorState -> [String] -> [String] -> [String]
            -> IO (Either AsmError String)
handleIfdef state rest acc tokens = 
  case tokens of
    [_, name] -> 
      let cond = M.member name (ppDefines state)
          newState = state { ppCondStack = cond : ppCondStack state }
      in processLines newState rest acc
    _ -> return $ Left $ AsmError 
      (SourceLoc (ppCurrentFile state) 0 0)
      "Invalid %ifdef directive"

handleIfndef :: PreprocessorState -> [String] -> [String] -> [String]
             -> IO (Either AsmError String)
handleIfndef state rest acc tokens = 
  case tokens of
    [_, name] -> 
      let cond = not $ M.member name (ppDefines state)
          newState = state { ppCondStack = cond : ppCondStack state }
      in processLines newState rest acc
    _ -> return $ Left $ AsmError 
      (SourceLoc (ppCurrentFile state) 0 0)
      "Invalid %ifndef directive"

handleElif :: PreprocessorState -> [String] -> [String]
           -> IO (Either AsmError String)
handleElif state rest acc = 
  case ppCondStack state of
    [] -> return $ Left $ AsmError 
      (SourceLoc (ppCurrentFile state) 0 0)
      "%elif without %if"
    (c:cs) -> 
      let newState = state { ppCondStack = not c : cs }
      in processLines newState rest acc

handleElse :: PreprocessorState -> [String] -> [String]
           -> IO (Either AsmError String)
handleElse state rest acc = 
  case ppCondStack state of
    [] -> return $ Left $ AsmError 
      (SourceLoc (ppCurrentFile state) 0 0)
      "%else without %if"
    (c:cs) -> 
      let newState = state { ppCondStack = not c : cs }
      in processLines newState rest acc

handleEndif :: PreprocessorState -> [String] -> [String]
            -> IO (Either AsmError String)
handleEndif state rest acc = 
  case ppCondStack state of
    [] -> return $ Left $ AsmError 
      (SourceLoc (ppCurrentFile state) 0 0)
      "%endif without %if"
    (_:cs) -> 
      let newState = state { ppCondStack = cs }
      in processLines newState rest acc

handleRep :: PreprocessorState -> [String] -> [String] -> String
          -> IO (Either AsmError String)
handleRep state rest acc countStr = 
  let count = read (filter isDigit countStr) :: Int
      (body, afterRep) = extractRepBody rest []
      expanded = concat $ replicate count (unlines body)
  in processLines state afterRep (expanded : acc)
  where
    isDigit c = c >= '0' && c <= '9'

extractRepBody :: [String] -> [String] -> ([String], [String])
extractRepBody [] acc = (reverse acc, [])
extractRepBody (line:rest) acc
  | "%endrep" `isPrefixOf` dropWhile (== ' ') line = (reverse acc, rest)
  | otherwise = extractRepBody rest (line : acc)

evaluateCondition :: PreprocessorState -> String -> Bool
evaluateCondition state cond = 
  case words cond of
    [n] -> case M.lookup n (ppDefines state) of
      Just v -> v /= 0
      Nothing -> False
    _ -> True
