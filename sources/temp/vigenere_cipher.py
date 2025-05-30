# Function to decrypt a Vigenère cipher
def vigenere_decrypt(cipher_text, keyword):
    print("Inside the decryption function...")  # Debugging: Verify the function is called
    decrypted_text = ""
    keyword_repeated = (keyword * ((len(cipher_text) // len(keyword)) + 1)).lower()
    keyword_index = 0

    for char in cipher_text:
        print(f"Processing character: {char}")  # Debugging: Check each character
        if char.isalpha():
            # Calculate the shift based on the keyword
            shift = ord(keyword_repeated[keyword_index]) - ord('a')
            
            # Debug: Print the shift value for each letter
            print(f"Shift for '{char}' using keyword letter '{keyword_repeated[keyword_index]}': {shift}")

            # Decrypt uppercase letters
            if char.isupper():
                decrypted_text += chr((ord(char) - shift - 65) % 26 + 65)
            
            # Decrypt lowercase letters
            elif char.islower():
                decrypted_text += chr((ord(char) - shift - 97) % 26 + 97)

            # Move to the next character in the keyword
            keyword_index += 1
        else:
            # Non-alphabetic characters remain the same
            decrypted_text += char

    print(f"Decrypted text so far: {decrypted_text}")  # Debugging: Check decrypted text progress
    return decrypted_text

# Hard-code the values for testing
cipher_text = "Lxfopv ef rnhr"
keyword = "lemon"

print(f"Cipher text: {cipher_text}")  # Debugging: Verify input values
print(f"Keyword: {keyword}")  # Debugging: Verify input values

# Decrypt and display the message
decrypted_message = vigenere_decrypt(cipher_text, keyword)
print(f"Decrypted Message: {decrypted_message}")  # Final output message

# Pause to view the result
input("Press Enter to exit...")
