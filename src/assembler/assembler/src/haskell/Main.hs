module Main where

import System.Environment
import System.Exit
import Frontend

main :: IO ()
main = do
  args <- getArgs
  case args of
    [inputFile, outputBase] -> do
      result <- compileFile inputFile (outputBase ++ ".o")
      case result of
        Left err -> do
          putStrLn err
          exitFailure
        Right () -> do
          putStrLn $ "Successfully assembled: " ++ outputBase ++ ".o"
          exitSuccess
    _ -> do
      putStrLn "Usage: crasm <input.asm> <output>"
      putStrLn "  Creates output.o object file"
      exitFailure
