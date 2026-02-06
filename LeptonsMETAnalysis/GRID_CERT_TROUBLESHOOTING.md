# Grid Certificate Troubleshooting Guide

**Issue**: `voms-proxy-init` fails with "bad decrypt" / "wrong password" errors

---

## Quick Diagnosis

Run these commands to check your certificate status:

```bash
# 1. Check if certificate files exist
ls -la ~/.globus/

# 2. Check certificate file permissions (should be 400 for userkey.pem, 644 for usercert.pem)
ls -la ~/.globus/*.pem

# 3. Check certificate expiration
openssl x509 -in ~/.globus/usercert.pem -noout -dates

# 4. Check if password file exists (not recommended, but some people use it)
ls -la ~/.globus/.password 2>/dev/null || echo "No password file (this is OK)"
```

---

## Common Causes and Solutions

### Cause 1: Wrong Password ⭐ (Most Common)

**Symptom**: You're entering a password, but it's not being accepted.

**Important**: The password for your grid certificate is **NOT** your CERN password. It's a separate password you set when you created the certificate.

**Solution**:
1. **Try to remember the password** you set when creating the certificate (possibly years ago)
2. Common passwords people use:
   - The CERN password from when they created the cert
   - A specific "grid" password they set
   - Written down somewhere in old notes
3. **If you can't remember**: You'll need to renew the certificate (see below)

**Try this first**:
```bash
# Try entering the password very carefully
voms-proxy-init -voms atlas -valid 168:00
# Enter password slowly and carefully
```

---

### Cause 2: Certificate Expired

**Check expiration**:
```bash
openssl x509 -in ~/.globus/usercert.pem -noout -dates
```

**Expected output**:
```
notBefore=Jan 15 10:00:00 2024 GMT
notAfter=Jan 14 10:00:00 2025 GMT
```

**If expired** (notAfter is in the past):
- You must renew your certificate (see "How to Renew Certificate" below)

---

### Cause 3: Corrupted Certificate Files

**Check file integrity**:
```bash
# Check if usercert.pem is valid
openssl x509 -in ~/.globus/usercert.pem -noout -text

# Check if userkey.pem is valid (will ask for password)
openssl rsa -in ~/.globus/userkey.pem -check
```

**If files are corrupted**:
- You may have a backup in `~/.globus/.backup/`
- Or you'll need to renew the certificate

---

### Cause 4: Wrong File Permissions

**Check permissions**:
```bash
ls -la ~/.globus/*.pem
```

**Expected**:
```
-r--------  1 alpizzin zp  1704 Jan 15  2024 userkey.pem   <- 400
-rw-r--r--  1 alpizzin zp  1830 Jan 15  2024 usercert.pem  <- 644
```

**Fix permissions**:
```bash
chmod 400 ~/.globus/userkey.pem
chmod 644 ~/.globus/usercert.pem
```

Then try again:
```bash
voms-proxy-init -voms atlas -valid 168:00
```

---

### Cause 5: Files in Wrong Format

CERN now issues certificates as `.p12` files, but the old format was separate `.pem` files.

**Check if you have a .p12 file**:
```bash
ls -la ~/.globus/*.p12
```

**If you have a .p12 file**, you may need to convert it:
```bash
# Extract the certificate
openssl pkcs12 -in ~/.globus/myCertificate.p12 -clcerts -nokeys -out ~/.globus/usercert.pem

# Extract the key
openssl pkcs12 -in ~/.globus/myCertificate.p12 -nocerts -out ~/.globus/userkey.pem

# Set correct permissions
chmod 644 ~/.globus/usercert.pem
chmod 400 ~/.globus/userkey.pem
```

---

## How to Renew Your Certificate

### Option 1: Renew at CERN

1. **Go to CERN Certification Authority**:
   - https://ca.cern.ch/ca/

2. **Log in** with your CERN credentials

3. **Request New User Certificate**:
   - Click "New Grid User Certificate"
   - Follow the wizard
   - You'll set a **new password** (write it down!)
   - Download the `.p12` file

4. **Install the certificate**:
   ```bash
   # Backup old certificates
   mkdir -p ~/.globus/.backup
   mv ~/.globus/*.pem ~/.globus/.backup/

   # Convert the new .p12 file
   openssl pkcs12 -in ~/Downloads/myCertificate.p12 -clcerts -nokeys -out ~/.globus/usercert.pem
   openssl pkcs12 -in ~/Downloads/myCertificate.p12 -nocerts -out ~/.globus/userkey.pem

   # Set permissions
   chmod 644 ~/.globus/usercert.pem
   chmod 400 ~/.globus/userkey.pem
   ```

5. **Test**:
   ```bash
   voms-proxy-init -voms atlas -valid 168:00
   # Use the NEW password you just set
   ```

---

### Option 2: Check for Backup Certificates

**Look for backups**:
```bash
# Check for backup directory
ls -la ~/.globus/.backup/

# Check AFS backup (if you're on lxplus)
ls -la ~/.globus/.backup.*/
```

**If you find backups**:
```bash
# Copy back (adjust date to match your backup)
cp ~/.globus/.backup/usercert.pem ~/.globus/
cp ~/.globus/.backup/userkey.pem ~/.globus/

# Fix permissions
chmod 644 ~/.globus/usercert.pem
chmod 400 ~/.globus/userkey.pem

# Try the OLD password you used
voms-proxy-init -voms atlas -valid 168:00
```

---

## Emergency Workaround: Using Someone Else's Proxy (TEMPORARY ONLY)

**⚠️ NOT RECOMMENDED FOR PRODUCTION ⚠️**

If you need to work urgently while waiting for a new certificate, ask a colleague to create a proxy for you:

**On colleague's machine**:
```bash
voms-proxy-init -voms atlas -valid 168:00 -out /tmp/myproxy
# Send the file to you (e.g., scp to lxplus)
```

**On your machine**:
```bash
# Copy their proxy
export X509_USER_PROXY=/path/to/their/proxy

# This will work for read-only operations
# DO NOT use for grid submissions (will be under their name)
```

**This is ONLY for emergency read access. You MUST get your own certificate for any production work.**

---

## Step-by-Step Recovery Process

### Step 1: Diagnose the Issue

Run this diagnostic script:

```bash
cat > /tmp/check_cert.sh << 'EOF'
#!/bin/bash
echo "=== Grid Certificate Diagnostic ==="
echo ""
echo "1. Certificate files:"
ls -la ~/.globus/*.pem 2>/dev/null || echo "  No .pem files found"
ls -la ~/.globus/*.p12 2>/dev/null || echo "  No .p12 files found"
echo ""

echo "2. Certificate expiration:"
if [ -f ~/.globus/usercert.pem ]; then
    openssl x509 -in ~/.globus/usercert.pem -noout -dates
else
    echo "  usercert.pem not found"
fi
echo ""

echo "3. File permissions:"
if [ -f ~/.globus/userkey.pem ]; then
    PERM=$(stat -c %a ~/.globus/userkey.pem)
    echo "  userkey.pem: $PERM (should be 400)"
fi
if [ -f ~/.globus/usercert.pem ]; then
    PERM=$(stat -c %a ~/.globus/usercert.pem)
    echo "  usercert.pem: $PERM (should be 644)"
fi
echo ""

echo "4. Certificate validity:"
if [ -f ~/.globus/usercert.pem ]; then
    openssl x509 -in ~/.globus/usercert.pem -noout -subject -issuer
else
    echo "  Cannot check - usercert.pem missing"
fi
echo ""

echo "5. Backups:"
ls -la ~/.globus/.backup/*.pem 2>/dev/null || echo "  No backups found"
echo ""
EOF

chmod +x /tmp/check_cert.sh
/tmp/check_cert.sh
```

### Step 2: Based on Diagnostic Output

**If files exist and not expired**:
- Try password carefully (maybe from old notes)
- Check for password hints you may have written

**If files expired**:
- Renew certificate at https://ca.cern.ch/ca/

**If files missing or corrupted**:
- Check backups
- Renew certificate

**If permissions wrong**:
- Fix with chmod commands above

### Step 3: Verify Fix

```bash
# Try to create proxy
voms-proxy-init -voms atlas -valid 168:00

# Check proxy info
voms-proxy-info --all

# Expected output should include:
#   subject   : /DC=ch/DC=cern/OU=Organic Units/...
#   timeleft  : 167:59:59
```

---

## Contact Information

**If all else fails**:

1. **ATLAS Grid Support**:
   - Email: atlas-adc-grid-support@cern.ch
   - Explain your issue, include output of diagnostic script

2. **CERN CA Support**:
   - https://ca.cern.ch/ca/Help
   - For certificate renewal issues

3. **Your Local Grid Contact**:
   - Ask your institute's grid expert for help

---

## Prevention for the Future

1. **Write down your certificate password** in a secure location
2. **Set a calendar reminder** 6 months before expiration
3. **Keep a backup**:
   ```bash
   cp ~/.globus/userkey.pem ~/.globus/.backup/
   cp ~/.globus/usercert.pem ~/.globus/.backup/
   ```
4. **Use the same password** each time you renew (if you can remember it)

---

## Summary

Most common fix:
```bash
# 1. Fix permissions
chmod 400 ~/.globus/userkey.pem
chmod 644 ~/.globus/usercert.pem

# 2. Try proxy init
voms-proxy-init -voms atlas -valid 168:00
# Enter password CAREFULLY

# 3. If still fails, check expiration:
openssl x509 -in ~/.globus/usercert.pem -noout -dates

# 4. If expired or can't remember password:
#    Go to https://ca.cern.ch/ca/ and renew
```
