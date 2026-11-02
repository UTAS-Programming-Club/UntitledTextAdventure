import Data.String
import Data.Vect
import System.File.ReadWrite

-- Works on accepting a vec8 of db queries and overwriting the following db line with
-- 8 lines of a shifted transformation. Adds a bit of colour, wanted to automate this
-- because etc etc XKCD etc etc

-- i should've used haskell I'm just avoiding LLM based compilers here

eraseFirstOne : List Char -> List Char
eraseFirstOne ('1' :: xs) = '0' :: xs
eraseFirstOne (x   :: xs) = x :: (eraseFirstOne xs)
eraseFirstOne _           = []

-- take 8 lines, then drop next line and append lhs
collect : {n : Nat} -> List String -> Vect n String -> List String
collect { n = 8 } (_ :: xs') ext = 
    let extList = toList ext
        res = (map pack (map eraseFirstOne (map unpack extList)))
	in res ++ collect xs' Nil
collect (x :: xs') ext with (isPrefixOf ";" x, x == "")
	collect (x :: xs') ext | (True, _) = x :: collect xs' ext
	collect (x :: xs') ext | (_, True) = x :: collect xs' ext
	collect (x :: xs') ext | (False, False) = x :: collect xs' (ext ++ [x])
collect _ ext = 
    let extList = toList ext
        res = (map pack (map eraseFirstOne (map unpack extList)))
	in res

main : IO ()
main = do
	file <- readFile "font.asm"
	case file of
		Right n => putStrLn (fastUnlines (map (indent 4) (collect (map trim (lines n)) Nil)))
		Left err => putStrLn (show err)
