{-# LANGUAGE ForeignFunctionInterface #-}

module FFI where

import Foreign.C.Types

-- Stub for Rust backend
foreign export ccall "hs_init_wrapper" hsInitWrapper :: IO ()
foreign export ccall "hs_exit_wrapper" hsExitWrapper :: IO ()

hsInitWrapper :: IO ()
hsInitWrapper = return ()

hsExitWrapper :: IO ()
hsExitWrapper = return ()
