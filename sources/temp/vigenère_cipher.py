# Function to decrypt a Vigenère cipher
def vigenere_decrypt(cipher_text, keyword):
    decrypted_text = ""
    keyword_repeated = (keyword * ((len(cipher_text) // len(keyword)) + 1)).lower()
    keyword_index = 0

    for char in cipher_text:
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

    return decrypted_text

# Input: Encrypted message and keyword (Note the quotes around the inputs!)
cipher_text = input("Lxfopv ef rnhr")
keyword = input("lemon")

# Decrypt and display the message
decrypted_message = vigenere_decrypt(cipher_text, keyword)
print(f"Decrypted Message: {decrypted_message}")

# Pause to view the result
input("Press Enter to exit...")
