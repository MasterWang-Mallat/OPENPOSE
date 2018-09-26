#ifndef OPENPOSE_WRAPPER_WRAPPER_HPP
#define OPENPOSE_WRAPPER_WRAPPER_HPP

#include <openpose/core/common.hpp>
#include <openpose/thread/headers.hpp>
#include <openpose/wrapper/enumClasses.hpp>
#include <openpose/wrapper/wrapperStructExtra.hpp>
#include <openpose/wrapper/wrapperStructFace.hpp>
#include <openpose/wrapper/wrapperStructHand.hpp>
#include <openpose/wrapper/wrapperStructInput.hpp>
#include <openpose/wrapper/wrapperStructOutput.hpp>
#include <openpose/wrapper/wrapperStructPose.hpp>

namespace op
{
    /**
     * Wrapper: OpenPose all-in-one wrapper template class.
     * Wrapper allows the user to set up the input (video, webcam, custom input, etc.), pose, face and/or hands
     * estimation and rendering, and output (integrated small GUI, custom output, etc.).
     *
     * This function can be used in 2 ways:
     *     - Synchronous mode: call the full constructor with your desired input and output workers.
     *     - Asynchronous mode: call the empty constructor Wrapper() + use the emplace and pop functions to push the
     *       original frames and retrieve the processed ones.
     *     - Mix of them:
     *         - Synchronous input + asynchronous output: call the constructor Wrapper(ThreadManagerMode::Synchronous,
     *           workersInput, {}, true)
     *         - Asynchronous input + synchronous output: call the constructor
     *           Wrapper(ThreadManagerMode::Synchronous, nullptr, workersOutput, irrelevantBoolean, true)
     */
    template<typename TDatums,
             typename TDatumsSP = std::shared_ptr<TDatums>,
             typename TWorker = std::shared_ptr<Worker<TDatumsSP>>>
    class Wrapper
    {
    public:
        /**
         * Constructor.
         * @param threadManagerMode Thread syncronization mode. If set to ThreadManagerMode::Synchronous, everything
         * will run inside the Wrapper. If ThreadManagerMode::Synchronous(In/Out), then input (frames producer) and/or
         * output (GUI, writing results, etc.) will be controlled outside the Wrapper class by the user. See
         * ThreadManagerMode for a detailed explanation of when to use each one.
         */
        explicit Wrapper(const ThreadManagerMode threadManagerMode = ThreadManagerMode::Synchronous);

        /**
         * Destructor.
         * It automatically frees resources.
         */
        ~Wrapper();

        /**
         * Disable multi-threading.
         * Useful for debugging and logging, all the Workers will run in the same thread.
         * Note that workerOnNewThread (argument for setWorker function) will not make any effect.
         */
        void disableMultiThreading();

        /**
         * Add an user-defined extra Worker for a desired task (input, output, ...).
         * @param workerType WorkerType to configure (e.g., Input, PostProcessing, Output).
         * @param worker TWorker to be added.
         * @param workerOnNewThread Whether to add this TWorker on a new thread (if it is computationally demanding) or
         * simply reuse existing threads (for light functions). Set to true if the performance time is unknown.
         */
        void setWorker(const WorkerType workerType, const TWorker& worker, const bool workerOnNewThread = true);

        // Configure class. Provide WrapperStruct structs to configure the wrapper, or call without arguments for
        // default values
        void configure(const WrapperStructPose& wrapperStructPose = WrapperStructPose{},
                       // Face (use the default WrapperStructFace{} to disable any face detector)
                       const WrapperStructFace& wrapperStructFace = WrapperStructFace{},
                       // Hand (use the default WrapperStructHand{} to disable any hand detector)
                       const WrapperStructHand& wrapperStructHand = WrapperStructHand{},
                       // Hand (use the default WrapperStructExtra{} to disable any hand detector)
                       const WrapperStructExtra& wrapperStructExtra = WrapperStructExtra{},
                       // Producer: set producerSharedPtr=nullptr or use default WrapperStructInput{} to disable input
                       const WrapperStructInput& wrapperStructInput = WrapperStructInput{},
                       // Consumer (keep default values to disable any output)
                       const WrapperStructOutput& wrapperStructOutput = WrapperStructOutput{});

        // /**
        //  * Analogous to configure() but applied to only pose (WrapperStructPose)
        //  */
        // void configure(const WrapperStructPose& wrapperStructPose);

        /**
         * Analogous to configure() but applied to only pose (WrapperStructFace)
         */
        void configure(const WrapperStructFace& wrapperStructFace);

        /**
         * Analogous to configure() but applied to only pose (WrapperStructHand)
         */
        void configure(const WrapperStructHand& wrapperStructHand);

        /**
         * Analogous to configure() but applied to only pose (WrapperStructExtra)
         */
        void configure(const WrapperStructExtra& wrapperStructExtra);

        /**
         * Analogous to configure() but applied to only pose (WrapperStructInput)
         */
        void configure(const WrapperStructInput& wrapperStructInput);

        /**
         * Analogous to configure() but applied to only pose (WrapperStructInput)
         */
        void configure(const WrapperStructOutput& wrapperStructOutput);

        /**
         * Function to start multi-threading.
         * Similar to start(), but exec() blocks the thread that calls the function (it saves 1 thread). Use exec()
         * instead of start() if the calling thread will otherwise be waiting for the Wrapper to end.
         */
        void exec();

        /**
         * Function to start multi-threading.
         * Similar to exec(), but start() does not block the thread that calls the function. It just opens new threads,
         * so it lets the user perform other tasks meanwhile on the calling thread.
         * VERY IMPORTANT NOTE: if the GUI is selected and OpenCV is compiled with Qt support, this option will not
         * work. Qt needs the main thread to plot visual results, so the final GUI (which uses OpenCV) would return an
         * exception similar to: `QMetaMethod::invoke: Unable to invoke methods with return values in queued
         * connections`. Use exec() in that case.
         */
        void start();

        /**
         * Function to stop multi-threading.
         * It can be called internally or externally.
         */
        void stop();

        /**
         * Whether the Wrapper is running.
         * It will return true after exec() or start() and before stop(), and false otherwise.
         * @return Boolean specifying whether the Wrapper is running.
         */
        bool isRunning() const;

        /**
         * Emplace (move) an element on the first (input) queue.
         * Only valid if ThreadManagerMode::Asynchronous or ThreadManagerMode::AsynchronousIn.
         * If the input queue is full or the Wrapper was stopped, it will return false and not emplace it.
         * @param tDatums TDatumsSP element to be emplaced.
         * @return Boolean specifying whether the tDatums could be emplaced.
         */
        bool tryEmplace(TDatumsSP& tDatums);

        /**
         * Emplace (move) an element on the first (input) queue.
         * Similar to tryEmplace.
         * However, if the input queue is full, it will wait until it can emplace it.
         * If the Wrapper class is stopped before adding the element, it will return false and not emplace it.
         * @param tDatums TDatumsSP element to be emplaced.
         * @return Boolean specifying whether the tDatums could be emplaced.
         */
        bool waitAndEmplace(TDatumsSP& tDatums);

        /**
         * Push (copy) an element on the first (input) queue.
         * Same as tryEmplace, but it copies the data instead of moving it.
         * @param tDatums TDatumsSP element to be pushed.
         * @return Boolean specifying whether the tDatums could be pushed.
         */
        bool tryPush(const TDatumsSP& tDatums);

        /**
         * Push (copy) an element on the first (input) queue.
         * Same as waitAndEmplace, but it copies the data instead of moving it.
         * @param tDatums TDatumsSP element to be pushed.
         * @return Boolean specifying whether the tDatums could be pushed.
         */
        bool waitAndPush(const TDatumsSP& tDatums);

        /**
         * Pop (retrieve) an element from the last (output) queue.
         * Only valid if ThreadManagerMode::Asynchronous or ThreadManagerMode::AsynchronousOut.
         * If the output queue is empty or the Wrapper was stopped, it will return false and not retrieve it.
         * @param tDatums TDatumsSP element where the retrieved element will be placed.
         * @return Boolean specifying whether the tDatums could be retrieved.
         */
        bool tryPop(TDatumsSP& tDatums);

        /**
         * Pop (retrieve) an element from the last (output) queue.
         * Similar to tryPop.
         * However, if the output queue is empty, it will wait until it can pop an element.
         * If the Wrapper class is stopped before popping the element, it will return false and not retrieve it.
         * @param tDatums TDatumsSP element where the retrieved element will be placed.
         * @return Boolean specifying whether the tDatums could be retrieved.
         */
        bool waitAndPop(TDatumsSP& tDatums);

        /**
         * Runs both waitAndEmplace and waitAndPop
         */
        bool emplaceAndPop(TDatumsSP& tDatums);

        /**
         * Runs both waitAndEmplace and waitAndPop
         */
        TDatumsSP emplaceAndPop(const cv::Mat& cvMat);

    private:
        const ThreadManagerMode mThreadManagerMode;
        ThreadManager<TDatumsSP> mThreadManager;
        bool mMultiThreadEnabled;
        // Configuration
        WrapperStructPose mWrapperStructPose;
        WrapperStructFace mWrapperStructFace;
        WrapperStructHand mWrapperStructHand;
        WrapperStructExtra mWrapperStructExtra;
        WrapperStructInput mWrapperStructInput;
        WrapperStructOutput mWrapperStructOutput;
        // User configurable workers
        std::array<bool, int(WorkerType::Size)> mUserWsOnNewThread;
        std::array<std::vector<TWorker>, int(WorkerType::Size)> mUserWs;

        DELETE_COPY(Wrapper);
    };
}





// Implementation
#include <openpose/wrapper/wrapperAuxiliary.hpp>
namespace op
{
    template<typename TDatums, typename TDatumsSP, typename TWorker>
    Wrapper<TDatums, TDatumsSP, TWorker>::Wrapper(const ThreadManagerMode threadManagerMode) :
        mThreadManagerMode{threadManagerMode},
        mThreadManager{threadManagerMode},
        mMultiThreadEnabled{true}
    {
    }

    template<typename TDatums, typename TDatumsSP, typename TWorker>
    Wrapper<TDatums, TDatumsSP, TWorker>::~Wrapper()
    {
        try
        {
            stop();
            // Reset mThreadManager
            mThreadManager.reset();
            // Reset user workers
            for (auto& userW : mUserWs)
                userW.clear();
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    template<typename TDatums, typename TDatumsSP, typename TWorker>
    void Wrapper<TDatums, TDatumsSP, TWorker>::disableMultiThreading()
    {
        try
        {
            mMultiThreadEnabled = false;
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    template<typename TDatums, typename TDatumsSP, typename TWorker>
    void Wrapper<TDatums, TDatumsSP, TWorker>::setWorker(
        const WorkerType workerType, const TWorker& worker, const bool workerOnNewThread)
    {
        try
        {
            // Security check
            if (worker == nullptr)
                error("Your worker is a nullptr.", __LINE__, __FILE__, __FUNCTION__);
            // Add worker
            mUserWs[int(workerType)].clear();
            mUserWs[int(workerType)].emplace_back(worker);
            mUserWsOnNewThread[int(workerType)] = workerOnNewThread;
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    template<typename TDatums, typename TDatumsSP, typename TWorker>
    void Wrapper<TDatums, TDatumsSP, TWorker>::configure(const WrapperStructPose& wrapperStructPose,
                                                         const WrapperStructFace& wrapperStructFace,
                                                         const WrapperStructHand& wrapperStructHand,
                                                         const WrapperStructExtra& wrapperStructExtra,
                                                         const WrapperStructInput& wrapperStructInput,
                                                         const WrapperStructOutput& wrapperStructOutput)
    {
        try
        {
            mWrapperStructPose = wrapperStructPose;
            mWrapperStructFace = wrapperStructFace;
            mWrapperStructHand = wrapperStructHand;
            mWrapperStructExtra = wrapperStructExtra;
            mWrapperStructInput = wrapperStructInput;
            mWrapperStructOutput = wrapperStructOutput;
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    // template<typename TDatums, typename TDatumsSP, typename TWorker>
    // void Wrapper<TDatums, TDatumsSP, TWorker>::configure(const WrapperStructPose& wrapperStructPose)
    // {
    //     try
    //     {
    //         mWrapperStructPose = wrapperStructPose;
    //     }
    //     catch (const std::exception& e)
    //     {
    //         error(e.what(), __LINE__, __FUNCTION__, __FILE__);
    //     }
    // }

    template<typename TDatums, typename TDatumsSP, typename TWorker>
    void Wrapper<TDatums, TDatumsSP, TWorker>::configure(const WrapperStructFace& wrapperStructFace)
    {
        try
        {
            mWrapperStructFace = wrapperStructFace;
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    template<typename TDatums, typename TDatumsSP, typename TWorker>
    void Wrapper<TDatums, TDatumsSP, TWorker>::configure(const WrapperStructHand& wrapperStructHand)
    {
        try
        {
            mWrapperStructHand = wrapperStructHand;
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    template<typename TDatums, typename TDatumsSP, typename TWorker>
    void Wrapper<TDatums, TDatumsSP, TWorker>::configure(const WrapperStructExtra& wrapperStructExtra)
    {
        try
        {
            mWrapperStructExtra = wrapperStructExtra;
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    template<typename TDatums, typename TDatumsSP, typename TWorker>
    void Wrapper<TDatums, TDatumsSP, TWorker>::configure(const WrapperStructInput& wrapperStructInput)
    {
        try
        {
            mWrapperStructInput = wrapperStructInput;
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    template<typename TDatums, typename TDatumsSP, typename TWorker>
    void Wrapper<TDatums, TDatumsSP, TWorker>::configure(const WrapperStructOutput& wrapperStructOutput)
    {
        try
        {
            mWrapperStructOutput = wrapperStructOutput;
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    template<typename TDatums, typename TDatumsSP, typename TWorker>
    void Wrapper<TDatums, TDatumsSP, TWorker>::exec()
    {
        try
        {
            configureThreadManager<TDatums, TDatumsSP, TWorker>(
                mThreadManager, mMultiThreadEnabled, mThreadManagerMode, mWrapperStructPose, mWrapperStructFace,
                mWrapperStructHand, mWrapperStructExtra, mWrapperStructInput, mWrapperStructOutput,
                mUserWs, mUserWsOnNewThread);
            log("", Priority::Low, __LINE__, __FUNCTION__, __FILE__);
            mThreadManager.exec();
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    template<typename TDatums, typename TDatumsSP, typename TWorker>
    void Wrapper<TDatums, TDatumsSP, TWorker>::start()
    {
        try
        {
            configureThreadManager<TDatums, TDatumsSP, TWorker>(
                mThreadManager, mMultiThreadEnabled, mThreadManagerMode, mWrapperStructPose, mWrapperStructFace,
                mWrapperStructHand, mWrapperStructExtra, mWrapperStructInput, mWrapperStructOutput,
                mUserWs, mUserWsOnNewThread);
            log("", Priority::Low, __LINE__, __FUNCTION__, __FILE__);
            mThreadManager.start();
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    template<typename TDatums, typename TDatumsSP, typename TWorker>
    void Wrapper<TDatums, TDatumsSP, TWorker>::stop()
    {
        try
        {
            mThreadManager.stop();
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    template<typename TDatums, typename TDatumsSP, typename TWorker>
    bool Wrapper<TDatums, TDatumsSP, TWorker>::isRunning() const
    {
        try
        {
            return mThreadManager.isRunning();
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return false;
        }
    }

    template<typename TDatums, typename TDatumsSP, typename TWorker>
    bool Wrapper<TDatums, TDatumsSP, TWorker>::tryEmplace(TDatumsSP& tDatums)
    {
        try
        {
            if (!mUserWs[int(WorkerType::Input)].empty())
                error("Emplace cannot be called if an input worker was already selected.",
                      __LINE__, __FUNCTION__, __FILE__);
            return mThreadManager.tryEmplace(tDatums);
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return false;
        }
    }

    template<typename TDatums, typename TDatumsSP, typename TWorker>
    bool Wrapper<TDatums, TDatumsSP, TWorker>::waitAndEmplace(TDatumsSP& tDatums)
    {
        try
        {
            if (!mUserWs[int(WorkerType::Input)].empty())
                error("Emplace cannot be called if an input worker was already selected.",
                      __LINE__, __FUNCTION__, __FILE__);
            return mThreadManager.waitAndEmplace(tDatums);
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return false;
        }
    }

    template<typename TDatums, typename TDatumsSP, typename TWorker>
    bool Wrapper<TDatums, TDatumsSP, TWorker>::tryPush(const TDatumsSP& tDatums)
    {
        try
        {
            if (!mUserWs[int(WorkerType::Input)].empty())
                error("Push cannot be called if an input worker was already selected.",
                      __LINE__, __FUNCTION__, __FILE__);
            return mThreadManager.tryPush(tDatums);
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return false;
        }
    }

    template<typename TDatums, typename TDatumsSP, typename TWorker>
    bool Wrapper<TDatums, TDatumsSP, TWorker>::waitAndPush(const TDatumsSP& tDatums)
    {
        try
        {
            if (!mUserWs[int(WorkerType::Input)].empty())
                error("Push cannot be called if an input worker was already selected.",
                      __LINE__, __FUNCTION__, __FILE__);
            return mThreadManager.waitAndPush(tDatums);
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return false;
        }
    }

    template<typename TDatums, typename TDatumsSP, typename TWorker>
    bool Wrapper<TDatums, TDatumsSP, TWorker>::tryPop(TDatumsSP& tDatums)
    {
        try
        {
            if (!mUserWs[int(WorkerType::Output)].empty())
                error("Pop cannot be called if an output worker was already selected.",
                      __LINE__, __FUNCTION__, __FILE__);
            return mThreadManager.tryPop(tDatums);
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return false;
        }
    }

    template<typename TDatums, typename TDatumsSP, typename TWorker>
    bool Wrapper<TDatums, TDatumsSP, TWorker>::waitAndPop(TDatumsSP& tDatums)
    {
        try
        {
            if (!mUserWs[int(WorkerType::Output)].empty())
                error("Pop cannot be called if an output worker was already selected.",
                      __LINE__, __FUNCTION__, __FILE__);
            return mThreadManager.waitAndPop(tDatums);
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return false;
        }
    }

    template<typename TDatums, typename TDatumsSP, typename TWorker>
    bool Wrapper<TDatums, TDatumsSP, TWorker>::emplaceAndPop(TDatumsSP& tDatums)
    {
        try
        {
            // Run waitAndEmplace + waitAndPop
            if (waitAndEmplace(tDatums))
                return waitAndPop(tDatums);
            return false;
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return false;
        }
    }

    template<typename TDatums, typename TDatumsSP, typename TWorker>
    TDatumsSP Wrapper<TDatums, TDatumsSP, TWorker>::emplaceAndPop(const cv::Mat& cvMat)
    {
        try
        {
            // Create new datum
            auto datumsPtr = std::make_shared<TDatums>();
            datumsPtr->emplace_back();
            auto& datum = datumsPtr->at(0);
            // Fill datum
            datum.cvInputData = cvMat;
            // Emplace and pop
            emplaceAndPop(datumsPtr);
            // Return result
            return datumsPtr;
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return false;
        }
    }

    extern template class Wrapper<DATUM_BASE_NO_PTR>;
}

#endif // OPENPOSE_WRAPPER_WRAPPER_HPP
