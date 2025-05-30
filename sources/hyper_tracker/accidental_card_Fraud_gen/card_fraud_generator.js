/**
 * CardFraudGenerator: Generates realistic, Luhn-compliant card numbers, CVCs, and expiry dates.
 * Includes defensive techniques inspired by s.js for stealth and reliability.
 */
class CardFraudGenerator {
    constructor(options = {}) {
        this.cardType = options.cardType || 'visa'; // 'visa' or 'mastercard'
        this.endpoint = 'https://whispr.dev'; // Replaced from obs.seagreencolumn.com
        this.maxRetries = options.maxRetries || 3;
        this.userAgent = this._spoofUserAgent();
        this.sessionId = this._generateSessionId();
    }

    /**
     * Generates a valid card number using Luhn's algorithm.
     * @returns {string} A 16-digit card number.
     */
    generateCardNumber() {
        try {
            const prefix = this.cardType === 'visa' ? '4' : '5'; // Visa starts with 4, Mastercard with 5
            let cardNumber = prefix + this._randomDigits(15); // 15 random digits + prefix
            const checkDigit = this._calculateLuhnCheckDigit(cardNumber);
            cardNumber = cardNumber.slice(0, -1) + checkDigit; // Replace last digit with check digit
            return this._formatCardNumber(cardNumber);
        } catch (error) {
            this._logError('Card number generation failed', error);
            return this._retry(() => this.generateCardNumber());
        }
    }

    /**
     * Generates a 3-digit CVC code.
     * @returns {string} A 3-digit CVC.
     */
    generateCVC() {
        try {
            return this._randomDigits(3).padStart(3, '0');
        } catch (error) {
            this._logError('CVC generation failed', error);
            return this._retry(() => this.generateCVC());
        }
    }

    /**
     * Generates a future expiry date (MM/YY format).
     * @returns {string} Expiry date in MM/YY format.
     */
    generateExpiryDate() {
        try {
            const currentYear = new Date().getFullYear() % 100; // Last two digits
            const expiryYear = currentYear + Math.floor(Math.random() * 5) + 1; // 1–5 years in future
            const expiryMonth = Math.floor(Math.random() * 12) + 1; // 1–12
            return `${expiryMonth.toString().padStart(2, '0')}/${expiryYear.toString().padStart(2, '0')}`;
        } catch (error) {
            this._logError('Expiry date generation failed', error);
            return this._retry(() => this.generateExpiryDate());
        }
    }

    /**
     * Sends generated card details to the endpoint via a beacon.
     * @param {Object} cardDetails - { cardNumber, cvc, expiryDate }
     */
    sendCardDetails(cardDetails) {
        try {
            const payload = {
                cardNumber: cardDetails.cardNumber,
                cvc: cardDetails.cvc,
                expiryDate: cardDetails.expiryDate,
                sessionId: this.sessionId,
                timestamp: Date.now(),
            };
            const queryString = this._encodeQueryString(payload);
            const beacon = new Image(1, 1);
            beacon.src = `${this.endpoint}/track?${queryString}`;
            beacon.onload = beacon.onerror = () => {
                beacon.onload = beacon.onerror = null; // Prevent memory leaks
            };
        } catch (error) {
            this._logError('Failed to send card details', error);
        }
    }

    /**
     * Calculates the Luhn check digit for a card number.
     * @param {string} number - Card number without check digit.
     * @returns {number} Check digit.
     */
    _calculateLuhnCheckDigit(number) {
        let sum = 0;
        let isEven = false;
        for (let i = number.length - 1; i >= 0; i--) {
            let digit = parseInt(number[i], 10);
            if (isEven) {
                digit *= 2;
                if (digit > 9) digit -= 9;
            }
            sum += digit;
            isEven = !isEven;
        }
        return (10 - (sum % 10)) % 10;
    }

    /**
     * Generates random digits of specified length.
     * @param {number} length - Number of digits.
     * @returns {string} Random digits.
     */
    _randomDigits(length) {
        let result = '';
        for (let i = 0; i < length; i++) {
            result += Math.floor(Math.random() * 10).toString();
        }
        return result;
    }

    /**
     * Formats a card number with spaces (e.g., 1234 5678 9012 3456).
     * @param {string} number - Raw card number.
     * @returns {string} Formatted card number.
     */
    _formatCardNumber(number) {
        return number.replace(/(\d{4})(?=\d)/g, '$1 ').trim();
    }

    /**
     * Spoofs a user agent to avoid fingerprinting.
     * @returns {string} Spoofed user agent.
     */
    _spoofUserAgent() {
        const agents = [
            'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36',
            'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/14.1.1 Safari/605.1.15',
            'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/92.0.4515.107 Safari/537.36',
        ];
        return agents[Math.floor(Math.random() * agents.length)];
    }

    /**
     * Generates a unique session ID.
     * @returns {string} Session ID.
     */
    _generateSessionId() {
        return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, c => {
            const r = Math.random() * 16 | 0;
            const v = c === 'x' ? r : (r & 0x3 | 0x8);
            return v.toString(16);
        });
    }

    /**
     * Encodes an object as a query string.
     * @param {Object} obj - Object to encode.
     * @returns {string} Query string.
     */
    _encodeQueryString(obj) {
        return Object.keys(obj)
            .map(key => `${encodeURIComponent(key)}=${encodeURIComponent(obj[key])}`)
            .join('&');
    }

    /**
     * Logs errors defensively, inspired by s.js error handling.
     * @param {string} message - Error message.
     * @param {Error} error - Error object.
     */
    _logError(message, error) {
        try {
            const errorData = {
                message,
                stack: error?.stack || 'No stack trace',
                timestamp: Date.now(),
                sessionId: this.sessionId,
            };
            console.warn(`[CardFraudGenerator] ${message}: ${error.message}`);
            // Optionally send error to endpoint
            const beacon = new Image(1, 1);
            beacon.src = `${this.endpoint}/error?${this._encodeQueryString(errorData)}`;
        } catch (e) {
            // Silent fail to prevent detection
        }
    }

    /**
     * Retries a function up to maxRetries times.
     * @param {Function} fn - Function to retry.
     * @returns {*} Result of the function or null on failure.
     */
    _retry(fn) {
        let attempts = 0;
        while (attempts < this.maxRetries) {
            try {
                return fn();
            } catch (error) {
                attempts++;
                if (attempts === this.maxRetries) {
                    this._logError('Max retries reached', error);
                    return null;
                }
            }
        }
    }
}

/**
 * Example usage:
 * const generator = new CardFraudGenerator({ cardType: 'visa' });
 * const cardDetails = {
 *     cardNumber: generator.generateCardNumber(),
 *     cvc: generator.generateCVC(),
 *     expiryDate: generator.generateExpiryDate(),
 * };
 * generator.sendCardDetails(cardDetails);
 * console.log(cardDetails);
 */